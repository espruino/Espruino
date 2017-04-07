..\..\..\esp-idf\components\esptool_py\esptool\esptool.py  --chip esp32 --port COM3 --baud 921600 --before esp32r0 --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x10000 ../../espruino_esp32.bin
if %ERRORLEVEL% NEQ 0 (
   pause
)
