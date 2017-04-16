..\..\..\esp-idf\components\esptool_py\esptool\esptool.py  --chip esp32  --baud 921600 --before esp32r0 --after hard_reset --port COM3 --baud 921600 erase_flash
if %ERRORLEVEL% NEQ 0 (
   pause
)
