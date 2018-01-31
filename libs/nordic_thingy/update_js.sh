#!/bin/bash
node ../../../EspruinoDocs/bin/minify.js Thingy.js Thingy.min.js
wget https://www.espruino.com/modules/LIS2DH12.min.js  -O LIS2DH12.min.js
wget https://www.espruino.com/modules/LPS22HB.min.js  -O LPS22HB.min.js
wget https://www.espruino.com/modules/HTS221.min.js  -O HTS221.min.js
wget https://www.espruino.com/modules/CCS811.min.js  -O CCS811.min.js
wget https://www.espruino.com/modules/BH1745.min.js  -O BH1745.min.js 
 
