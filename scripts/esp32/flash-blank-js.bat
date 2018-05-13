REM ..\..\..\esp-idf\components\esptool_py\esptool\esptool.py  --chip esp32 --port COM3 --baud 921600 --after soft_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x100000 blank.bin
..\..\..\esp-idf\components\esptool_py\esptool\esptool.py  --chip esp32 --port COM3 --baud 921600 erase_region 0x100000 0x10000
if %ERRORLEVEL% NEQ 0 (
   pause
)
