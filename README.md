# Teensyduino Installation
Teensyduino is the arduino plugin to enable programming Teensy development boards with the arduino IDE. Most all features and libraries are compatable.
1. Install the arduino IDE from the arduino website using installer or zip file. Do **NOT**  install the windows app from the app store. It will not work.
2. Download and run the Teensyduino software [https://www.pjrc.com/teensy/td_download.html](https://www.pjrc.com/teensy/td_download.html)
3. Follow the prompts and point the installer to where you installed the arduino IDE
4. Programming the Teensy can be a bit finicky. The software will direct you on what to do. If the device fails to enter programming mode the software is probably not correctly installed

# Huzzah 32
Huzzah 32 Boards just required the adddtion of a board manager. 
1. Download and install drivers [https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers)
2. Open arduino IDE preferences, and add the following to the "Additional Board Managers URLs" https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
3. Open Boards Manager from Tools > Board menu and install esp32 
