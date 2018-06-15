#!/bin/bash

cd `dirname $0`

# Get all JS modules we're using
wget https://www.espruino.com/modules/AT.min.js -O AT.min.js
#wget https://www.espruino.com/modules/QuectelM35.min.js -O QuectelM35.min.js
#wget https://www.espruino.com/modules/ATSMS.min.js -O ATSMS.min.js
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

# Other libs
wget https://www.espruino.com/modules/EspruinoWiFi.min.js -O espruino_wifi/Wifi.min.js
node ../../../EspruinoDocs/bin/minify.js nordic/Thingy.js nordic/Thingy.min.js
node ../../../EspruinoDocs/bin/minify.js rak/RAK8211.js rak/RAK8211.min.js

