#!/bin/bash

cd `dirname $0`

# Get all JS modules we're using
wget https://www.espruino.com/modules/AT.min.js -O AT.min.js
wget https://www.espruino.com/modules/QuectelM35.min.js -O QuectelM35.min.js
wget https://www.espruino.com/modules/QuectelBG96.min.js -O QuectelBG96.min.js
wget https://www.espruino.com/modules/ATSMS.min.js -O ATSMS.min.js
wget https://www.espruino.com/modules/GPS.min.js -O GPS.min.js
wget https://www.espruino.com/modules/graphical_menu.min.js -O graphical_menu.min.js
wget https://www.espruino.com/modules/BME280.min.js -O BME280.min.js
wget https://www.espruino.com/modules/LIS2DH12.min.js  -O LIS2DH12.min.js
wget https://www.espruino.com/modules/LIS2MDL.min.js  -O LIS2MDL.min.js
wget https://www.espruino.com/modules/LIS3DH.min.js  -O LIS3DH.min.js
wget https://www.espruino.com/modules/OPT3001.min.js  -O OPT3001.min.js
wget https://www.espruino.com/modules/MPU9250.min.js  -O MPU9250.min.js
wget https://www.espruino.com/modules/LPS22HB.min.js  -O LPS22HB.min.js
wget https://www.espruino.com/modules/HTS221.min.js  -O HTS221.min.js
wget https://www.espruino.com/modules/CCS811.min.js  -O CCS811.min.js
wget https://www.espruino.com/modules/BH1745.min.js  -O BH1745.min.js 
wget https://www.espruino.com/modules/SHT3C.min.js  -O SHT3C.min.js
#wget https://www.espruino.com/modules/PCA9685.min.js  -O PCA9685.min.js 
#wget https://www.espruino.com/modules/Smartibot.min.js  -O Smartibot.min.js 

# Other libs
wget https://www.espruino.com/modules/EspruinoWiFi.min.js -O espruino_wifi/Wifi.min.js
node ../../../EspruinoDocs/bin/minify.js nordic/Thingy.js nordic/Thingy.min.js
node ../../../EspruinoDocs/bin/minify.js rak/RAK8211.js rak/RAK8211.min.js
node ../../../EspruinoDocs/bin/minify.js rak/RAK8212.js rak/RAK8212.min.js
node ../../../EspruinoDocs/bin/minify.js nordic/Thingy.js nordic/Thingy.min.js

node ../../../EspruinoDocs/bin/minify.js pixljs/E_showMenu.js pixljs/E_showMenu.min.js
node ../../../EspruinoDocs/bin/minify.js pixljs/E_showMessage.js pixljs/E_showMessage.min.js
node ../../../EspruinoDocs/bin/minify.js pixljs/E_showPrompt.js pixljs/E_showPrompt.min.js
node ../../../EspruinoDocs/bin/minify.js pixljs/E_showAlert.js pixljs/E_showAlert.min.js
node ../../../EspruinoDocs/bin/minify.js banglejs/E_showMenu_F18.js banglejs/E_showMenu_F18.min.js
node ../../../EspruinoDocs/bin/minify.js banglejs/E_showMenu_Q3.js banglejs/E_showMenu_Q3.min.js
node ../../../EspruinoDocs/bin/minify.js banglejs/E_showMessage.js banglejs/E_showMessage.min.js
node ../../../EspruinoDocs/bin/minify.js banglejs/E_showPrompt.js banglejs/E_showPrompt.min.js
node ../../../EspruinoDocs/bin/minify.js banglejs/E_showPrompt_Q3.js banglejs/E_showPrompt_Q3.min.js
node ../../../EspruinoDocs/bin/minify.js banglejs/E_showAlert.js banglejs/E_showAlert.min.js
node ../../../EspruinoDocs/bin/minify.js banglejs/E_showScroller.js banglejs/E_showScroller.min.js
node ../../../EspruinoDocs/bin/minify.js banglejs/E_showScroller_Q3.js banglejs/E_showScroller_Q3.min.js
node ../../../EspruinoDocs/bin/minify.js banglejs/LED1.js banglejs/LED1.min.js
node ../../../EspruinoDocs/bin/minify.js banglejs/LED2.js banglejs/LED2.min.js
node ../../../EspruinoDocs/bin/minify.js banglejs/Bangle_drawWidgets.js banglejs/Bangle_drawWidgets.min.js
node ../../../EspruinoDocs/bin/minify.js banglejs/Bangle_drawWidgets_Q3.js banglejs/Bangle_drawWidgets_Q3.min.js
node ../../../EspruinoDocs/bin/minify.js banglejs/Bangle_loadWidgets.js banglejs/Bangle_loadWidgets.min.js
node ../../../EspruinoDocs/bin/minify.js banglejs/Bangle_showLauncher.js banglejs/Bangle_showLauncher.min.js
node ../../../EspruinoDocs/bin/minify.js banglejs/Bangle_showClock.js banglejs/Bangle_showClock.min.js
node ../../../EspruinoDocs/bin/minify.js banglejs/Bangle_load.js banglejs/Bangle_load.min.js
node ../../../EspruinoDocs/bin/minify.js banglejs/Bangle_setUI_F18.js banglejs/Bangle_setUI_F18.min.js
node ../../../EspruinoDocs/bin/minify.js banglejs/Bangle_setUI_Q3.js banglejs/Bangle_setUI_Q3.min.js
node ../../../EspruinoDocs/bin/minify.js banglejs/locale.js banglejs/locale.min.js


