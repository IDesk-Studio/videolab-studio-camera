# Использование формата ARGB32

Начиная с версии протокола 3, библиотека Softcam поддерживает альфа-канал через формат ARGB32.

## Создание камеры с альфа-каналом

```cpp
#include <softcam.h>

// Создание камеры с форматом ARGB32 (32-bit с альфа-каналом)
scCamera cam = scCreateCameraEx(640, 480, 60.0f, SC_PIXELFORMAT_ARGB32);

if (cam)
{
    // Создание буфера ARGB32 (4 байта на пиксель: A, R, G, B)
    const int width = 640;
    const int height = 480;
    const int bytes_per_pixel = 4; // ARGB32
    uint8_t* frame = new uint8_t[width * height * bytes_per_pixel];

    // Заполнение кадра с альфа-каналом
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int index = (y * width + x) * 4;

            frame[index + 0] = 255;  // B (Blue)
            frame[index + 1] = 0;    // G (Green)
            frame[index + 2] = 0;    // R (Red)
            frame[index + 3] = 128;  // A (Alpha) - полупрозрачность
        }
    }

    // Отправка кадра
    scSendFrame(cam, frame);

    delete[] frame;
    scDeleteCamera(cam);
}
```

## Сравнение форматов

### RGB24 (по умолчанию)
- 3 байта на пиксель (R, G, B)
- Размер кадра: `width × height × 3`
- Без альфа-канала
- Обратная совместимость с большинством приложений

```cpp
scCamera cam = scCreateCamera(640, 480, 60.0f);
// или явно:
scCamera cam = scCreateCameraEx(640, 480, 60.0f, SC_PIXELFORMAT_RGB24);
```

### ARGB32 (с альфа-каналом)
- 4 байта на пиксель (A, R, G, B)
- Размер кадра: `width × height × 4`
- Поддержка прозрачности
- Требует поддержки MEDIASUBTYPE_ARGB32 в приложении-потребителе

```cpp
scCamera cam = scCreateCameraEx(640, 480, 60.0f, SC_PIXELFORMAT_ARGB32);
```

## Формат данных

### RGB24 (bottom-up DIB)
```
Pixel layout: [B, G, R, B, G, R, ...]
Row stride: aligned to 4 bytes
```

### ARGB32 (bottom-up DIB)
```
Pixel layout: [B, G, R, A, B, G, R, A, ...]
Row stride: always 4-byte aligned (width × 4)
```

**Важно**: Оба формата используют bottom-up DIB формат (строки идут снизу вверх).

## Совместимость

- DirectShow фильтр автоматически определяет формат из shared memory
- Формат проверяется при подключении приложения-потребителя
- Старые версии Softcam (протокол v2 и ниже) не поддерживают ARGB32
- Не все камера-приложения поддерживают ARGB32 (большинство работают только с RGB24)

## Примечания

1. **Версия протокола**: ARGB32 требует ProtocolVersion = 3
2. **Обратная совместимость**: `scCreateCamera()` продолжает использовать RGB24 по умолчанию
3. **Производительность**: ARGB32 требует на 33% больше памяти и пропускной способности
4. **DirectShow**: Использует MEDIASUBTYPE_ARGB32 для 32-битного формата с валидным альфа-каналом
