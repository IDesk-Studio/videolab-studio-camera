# CLAUDE.md

Этот файл содержит руководство для Claude Code (claude.ai/code) при работе с кодом в этом репозитории.

## О проекте

Softcam - это библиотека для создания виртуальной веб-камеры на Windows. Реализована как DirectShow фильтр, который позволяет приложениям отправлять произвольные изображения в камеру-приложения через Softcam Sender API.

## Команды сборки и тестирования

### Сборка основной библиотеки
```bash
# Восстановление NuGet пакетов
nuget restore softcam.sln

# Сборка 64-битной версии (Release)
msbuild /m /p:Configuration=Release /p:Platform=x64 softcam.sln

# Сборка 32-битной версии (Release)
msbuild /m /p:Configuration=Release /p:Platform=Win32 softcam.sln

# Debug-сборка
msbuild /m /p:Configuration=Debug /p:Platform=x64 softcam.sln
```

Результаты сборки:
- DLL: `x64/Release/softcam.dll` или `Win32/Release/softcam.dll`
- Заголовки и import-библиотеки копируются в `dist/`

### Запуск тестов
```bash
# Unit-тесты (Google Test)
x64/Release/core_tests.exe

# Интеграционные тесты DLL
x64/Release/dll_tests.exe

# Запуск всех тестов для конкретной конфигурации
x64/Release/core_tests.exe && x64/Release/dll_tests.exe
```

### Сборка примеров
```bash
# Демо-приложение sender
msbuild /m /p:Configuration=Release /p:Platform=x64 examples/sender/sender.sln

# Установщик/деинсталлятор
msbuild /m /p:Configuration=Release /p:Platform=x64 examples/softcam_installer/softcam_installer.sln

# Python binding (только x64 Release)
cd examples/python_binding
call DownloadPybind11.bat
call _GetPythonPath.bat
msbuild /m /p:Configuration=Release /p:Platform=x64 python_binding.sln

# Тестирование Python binding
pip install pytest numpy
pytest
```

### Регистрация/удаление DLL в системе
```bash
# Регистрация 64-битной версии (требует прав администратора)
cd examples/softcam_installer
RegisterSoftcam.bat

# Регистрация 32-битной версии
RegisterSoftcam32.bat

# Удаление из системы
UnregisterSoftcam.bat      # 64-bit
UnregisterSoftcam32.bat    # 32-bit
```

## Архитектура проекта

### Структура компонентов

Проект состоит из трех основных библиотечных компонентов в `src/`:

#### 1. **src/baseclasses/** - Базовые классы DirectShow
- Официальные базовые классы Microsoft DirectShow (из Windows-classic-samples)
- Предоставляет фундаментальную инфраструктуру для фильтров DirectShow
- Компилируется как статическая библиотека
- Ключевые классы: `CSource`, `CSourceStream`, `CBaseFilter`, `CBasePin`

#### 2. **src/softcamcore/** - Ядро виртуальной камеры
Статическая библиотека, содержащая основную функциональность:

- **FrameBuffer.h/cpp**: Межпроцессный разделяемый буфер кадров
  - Управляет RGB24 видеокадрами (3 байта на пиксель)
  - Реализует коммуникацию между 32/64-битными процессами через Windows named mutexes и file mapping
  - Поддержка версионирования протокола (текущая версия 2)

- **DShowSoftcam.h/cpp**: Реализация DirectShow фильтра
  - Класс `Softcam`: реализует `CSource` + `IAMStreamConfig`
  - Класс `SoftcamStream`: реализует `CSourceStream` + `IKsPropertySet` + `IAMStreamConfig`
  - Обрабатывает согласование медиа-типов (RGB24, настраиваемое разрешение)
  - Метод `FillBuffer()` доставляет кадры в потребляющие приложения

- **SenderAPI.h/cpp**: Публичный API для отправителей кадров
  - Оборачивает операции FrameBuffer для производителей кадров
  - Управляет единственным экземпляром камеры на процесс через атомарные операции
  - Функции: `CreateCamera()`, `DeleteCamera()`, `SendFrame()`, `WaitForConnection()`, `IsConnected()`

- **Misc.h/cpp**: Низкоуровневые примитивы синхронизации
  - `Timer`: обертка над счетчиком производительности (Windows QPC)
  - `NamedMutex`: межпроцессный mutex через Windows named mutexes
  - `SharedMemory`: файлы с отображением памяти для IPC

- **Watchdog.h/cpp**: Мониторинг здоровья процессов
  - Обнаруживает сбои/отключения процессов через механизм таймаута watchdog
  - Heartbeat со стороны отправителя, мониторинг со стороны получателя

#### 3. **src/softcam/** - Точка входа DLL
- **softcam.h**: Публичный заголовок C API
  - Экспортирует 5 функций: `scCreateCamera`, `scDeleteCamera`, `scSendFrame`, `scWaitForConnection`, `scIsConnected`
  - C calling convention для языковых биндингов
  - Паттерн непрозрачного дескриптора (`scCamera = void*`)

- **softcam.cpp**: Реализация DLL
  - COM регистрация/дерегистрация
  - Регистрация фильтра в категории DirectShow Video Input Device
  - CLSID: {AEF3B972-5FA5-4647-9571-358EB472BC9E}

### Поток данных кадров

```
Приложение-отправитель (Sender)
        |
        v
scSendFrame() -> SenderAPI
        |
        v
FrameBuffer::write()
        |
        v
SharedMemory (named file mapping: "DirectShow Softcam/SharedMemory")
        |
        v
FrameBuffer::transferToDIB()
        |
        v
SoftcamStream::FillBuffer() <- DirectShow
        |
        v
Приложение-потребитель (Camera App)
```

### Межпроцессное взаимодействие (IPC)

Механизмы IPC для коммуникации 32/64-битных процессов:

1. **Named Mutex** (`"DirectShow Softcam/NamedMutex"`)
   - Синхронизация доступа к заголовку буфера кадров
   - Системный объект, работает между границами процессов

2. **File Mapping** (`"DirectShow Softcam/SharedMemory"`)
   - Хранение данных кадров + заголовка
   - Размер: `sizeof(Header) + width * height * 3`
   - Отправитель: `CreateFileMapping()` + `MapViewOfFile()`
   - Получатель: `OpenFileMapping()` + `MapViewOfFile()`

3. **Watchdog Heartbeats** (обнаружение сбоев)
   - Отправитель обновляет счетчик каждые 20ms
   - Получатель мониторит, таймаут при отсутствии изменений 500ms

### Структура тестов

#### tests/core_tests/ - Unit-тесты
Фреймворк: Google Test (gtest)

Тестовые файлы:
- `DShowSoftcamTest.cpp`: Тесты классов Softcam/SoftcamStream (COM интерфейсы, медиа-типы)
- `FrameBufferTest.cpp`: Тесты создания/чтения/записи буфера кадров
- `SenderAPITest.cpp`: Тесты sender API (CreateCamera, SendFrame, connections)
- `MiscTest.cpp`: Тесты Timer, NamedMutex, SharedMemory
- `WatchdogTest.cpp`: Тесты watchdog heartbeat/monitor

#### tests/dll_tests/ - Интеграционные тесты
Фреймворк: Google Test (gtest)

Тестовый файл: `raw_api_test.cpp`
- Тесты C API через динамическую линковку DLL
- Валидация создания/удаления камеры, невалидных аргументов
- Тесты обработки ошибок (double-free, невалидные указатели)

### Конфигурации сборки

- **Debug|Win32**: 32-битная debug DLL (`softcamd.dll`)
- **Debug|x64**: 64-битная debug DLL (`softcamd.dll`)
- **Release|Win32**: 32-битная release DLL (`softcam.dll`)
- **Release|x64**: 64-битная release DLL (`softcam.dll`)

Обе версии (32-битная и 64-битная) могут сосуществовать в системе и корректно взаимодействуют друг с другом.

## Ключевые файлы для понимания кодовой базы

1. **softcam/softcam.h** - публичная поверхность API
2. **softcamcore/SenderAPI.cpp** - механизм производства кадров
3. **softcamcore/DShowSoftcam.cpp** - потребление кадров через DirectShow
4. **softcamcore/FrameBuffer.cpp** - протокол IPC
5. **softcamcore/Misc.cpp** - примитивы синхронизации Windows

## Добавление новых функций

- **Новые опции framerate**: Модифицировать `DShowSoftcam.cpp::fillMediaType()`
- **Новые цветовые форматы**: Расширить `SoftcamStream::GetMediaType()`, обновить расчет размера FrameBuffer
- **Мониторинг соединения**: Улучшить `Watchdog` с дополнительными метриками
- **Оптимизация производительности**: Профилировать `FillBuffer()` и операции копирования кадров

## Отладка

- Включить логирование в `DShowSoftcam.cpp` (раскомментировать `#define ENABLE_LOG`)
- Проверить статус watchdog через `FrameBuffer::connected()`
- Проверить named mutex/file mapping в Windows Resource Monitor
- Тестировать с утилитой DirectShow GraphEdit для перечисления фильтров

## Зависимости

- Visual Studio 2022 (или 2019) с Desktop development with C++
- Windows 10 SDK
- Google Test (для тестов, через NuGet)
- Python 3.x + pybind11 (для python_binding)

## Примеры использования

- **examples/sender/**: Демо-приложение, отправляющее кадры с анимацией
- **examples/softcam_installer/**: Утилита для регистрации/дерегистрации COM
- **examples/python_binding/**: Python обертка для использования API из Python
