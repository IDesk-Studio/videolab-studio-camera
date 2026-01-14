@echo off

cls

net session >nul 2>&1
if %errorLevel% neq 0 (
    echo Run as admin!
    pause > nul
)

cd /d "%~dp0"

if not "%1" == "" (
    if not "%1" == "/u" (
        echo Unknown argument: %1
        echo Available arguments:
        echo - [none]: register DLLs from current directory
        echo - /u: unregister DLLs from current directory
        pause > nul
        exit 1
    )
)

for %%f in (*.dll) do (
    regsvr32 /s %1 "%%f"
)
