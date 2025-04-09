# Linux System Monitor - ESP32 and TFT-ILI9341

 ![alt text](/assets/imagen.jpg "ESP32 TFT")
 ![alt text](/assets/main_image.jpg "Bluetooth-System-Monitor")

 A handy little system information monitor using and ESP32 + ILI9488 TFT. Receives data over Serial Bluetooth and thus giving you the option to have it far away from your actual computer. On the computer side you can write your own program for writing over Serial BT, but I have provided python scripts for both Windows and MacOS. Instructions on how to install and use these are below.

## Connecting the screen
Use the image below to connect the screen to the ESP32.

 ![alt text](/assets/connecting.jpg "ESP32 TFT connections - uteh str")


## ESP32 side installation Linux (Ubuntu)

To use this sketch, you will need the Arduino IDE (1.8.10 or higher). You will also need to add the ESP32-Arduino core (https://github.com/espressif/arduino-esp32) to your board manager.

This sketch uses the latest version of TFT_eSPI, available here: https://github.com/Bodmer/TFT_eSPI

Before uploading your sketch, configure the TFT_eSPI library to use the correct pins. You'll need to edit the User_Setup.h file that comes with the TFT_eSPI library. You can use the configuration example in `user_setup.h Example` for this diagram, or use the one in `DustinWatts/Bluetooth-System-Monitor` for your connection diagram; there should be no problems either way: [https://github.com/DustinWatts/Bluetooth-System-Monitor/tree/main/user_setup.h Examples](https://github.com/DustinWatts/Bluetooth-System-Monitor/tree/main/user_setup.h%20Examples)

In the sketch, you can set some warning levels. This will turn the text read if above a certain value. In case of RAM, it will alert when it is under the warning value.

 ![alt text](/assets/warning_levels.png "Warning leves")

Open the sketch in the Arduino IDE. Select the right board, the right port and hit 'Upload'. After this, you will also need to upload the `data` folder. Because the background image also needs to be uploaded. You can use the "ESP Sketch Data Upload" tool that can be found here: https://github.com/me-no-dev/arduino-esp32fs-plugin
![ESP Sketch Data](http://dustinwatts.nl/freetouchdeck/images/ftd_esp_sketch_data.png)

## Computer side installation on Linux (Ubuntu):

On Ubuntu, the Python script uses PySerial to communicate with the ESP32 and uses psutil to get memory information and lm-sensors to get temperature information.

Install the requirements
`pip install -r requirements_windows.txt`

Or do it manually one by one:

`pip install pyserial`
Docs on PySerial: https://pythonhosted.org/pyserial/

`pip install psutil`
Docs on psutil: https://psutil.readthedocs.io/en/latest/

Get temperature with lm-sensors, although the way to obtain the temperature data will depend on the Linux distribution and you will have to adapt it in the script `linux_host.py`.

`sudo apt install lm-sensors`

_**Script python**_
Use `linux_host_UART.py` or `linux_host_BL.py` to send data

 ![alt text](/assets/port.png "Port")
 
 _**Code arduino**_
 
  ![alt text](/assets/mode.png "IDE Arduino")
 
 _**Running the script**_
 
 **This script needs administrator rights to get system info. And should be run from the command line as an Aministrator**
 To run: **`python linux_host_UART.py`** or **`python3 linux_host_UART.py`**

## Reference

This project is based on: 🔗 [https://github.com/DustinWatts/Bluetooth-System-Monitor](https://github.com/DustinWatts/Bluetooth-System-Monitor)

| Platform | Link|
|:-----:|:-----|
| [<img src="https://github.com/DustinWatts/small_logos/blob/main/twitter_logo.png" alt="Twtter" width="24"/>](https://twitter.com/dustinwattsnl "Follow me on Twitter") | You can follow me on Twitter: [@dustinwattsnl](https://twitter.com/dustinwattsnl "Follow me on Twitter")|
| [<img src="https://github.com/DustinWatts/small_logos/blob/main/youtube_logo.png" alt="YouTube" width="32"/>](https://www.youtube.com/dustinwatts "Subscrive to my YouTube channel") | You can subscribe to my channel on Youtube: [/dustinWatts](https://www.youtube.com/dustinwatts "Subscribe to my YouTube channel") |
| [<img src="https://github.com/DustinWatts/small_logos/blob/main/patreon_logo.png" alt="Patreon" width="32"/>](https://www.patreon.com/dustinwatts) | You can support me by becoming a patron on Patreon: https://www.patreon.com/dustinwatts |
| [<img src="https://github.com/DustinWatts/small_logos/blob/main/paypalme_logo.png" alt="PayPal.me" width="32"/>](https://www.paypal.me/dustinwattsnl) | You can make a one time donation using PayPal.me: https://www.paypal.me/dustinwattsnl |

