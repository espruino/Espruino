Using Nordic Bootloader
========================

Building complete hex file for Nordic DK upload
------------------------------------------------

Build the Bootloader:

```
make clean;PUCKJS=1 RELEASE=1 BOOTLOADER=1 make
```

Now build everything:

```
make clean;PUCKJS=1 RELEASE=1 make
```

Connect SWD connections and use:

```
PUCKJS=1 RELEASE=1 make flash
```

(Or just copy the zip file to the NRF52DK's flash drive)

Building zip file for Over the Air upload
-----------------------------------------

### Prep

https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v12.0.0%2Flib_crypto.html&anchor=lib_crypto_key

```
sudo apt install python-pip
pip install --upgrade pip
sudo pip install setuptools
sudo pip install nrfutil
```

### Compilation

```
make clean;PUCKJS=1 RELEASE=1 DFU_UPDATE_BUILD=1 make
```

# Flashing

* Download the `nRF Toolbox` app to your phone
* Download the ZIP file to a local folder on your phone
* Run the app and tap 'DFU'
* Select the file (Distribution Packet)
* Now take the battery out of the puck and re-insert it with the button hold down
* The Green LED should be lit - now release the button (under 3 secs after inserting battery)
* The Red LED should now be lit
* Tap 'select device' in the app, and choose `DfuTarg`
* Now tap `Upload`

Upload Failed: UNKNOWN (8202)
