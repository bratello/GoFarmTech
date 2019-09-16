# GoFarmTech Arduino based Framework for developing agriculture/industry process automation device.

This library provides a full feautured framework for development of serverless multifunction controller which able to controll process in unstable network connection environment. In some of the industry processes the price of network failure is very high - e.g. watering process in agriculture. So the device should be able to handle process independently from server, but allow to communicate with the MQTT server when it's possible. The main keys of the framework:

 - Set of the sensor and relay classes are ready to use
 - Simple sensor registration (see example)
 - Framework examines every sensor and posts the value change to the MQTT server (if possible right now)
 - Sensor supports the value change event for device logic implementation
 - Sensor supports min/max value limits and events when the value reaches the limits
 - Device can be configured through the Web UI independenty from the server (Initial network setup, sensor limits, value examination frequency, etc)
 - Ready to use inegration with ioBroker, which allows to controll and configure the device from the server: [ioBroker.gofarmtech](https://github.com/bratello/ioBroker.gofarmtech)
 - Logging system for maintenance and diagnostics

## Dependencies and Requirements
 - [Time](http://playground.arduino.cc/code/time): the device time will be updated from Web UI or from ioBroker automatically
 - [DHT sensor library](https://github.com/adafruit/DHT-sensor-library)
 - [PubSubClient](https://github.com/knolleary/pubsubclient): please install PubSubClient to the local lib folder and modify the PubSubClient::domain member's type from **const char\*** to the **String** (well known boggus pointer issue)
 - [Arduino_JSON](http://github.com/arduino-libraries/Arduino_JSON)
 - SPIFFS should be supported and available, in about 400K FS space is required.

## Build steps
 - Install all dependencies and requirements
 - Specify the data folder (data_dir in PlatformIO)
 - Create settings.txt & values.txt in data folder
 - Specify the following keys in settings.txt (without brackets or quotes, just a value after the equal sign)
    * deviceId=[Your device ID e.g.IoTMyConditionTimer - this ID will be used as Maintenance WiFi HotSpot name and as the MQTT channel ID]
    * deviceUsr=[Empty for now, for future use]
    * devicePwd=[Empty for now, for future use]
    * ssid=[WiFi network name for ioBroker connection]
    * wifiPwd=[WiFi password]
    * mqttHost=[MQTT Broker IP e.g. 192.168.0.120]
    * mqttPort=[MQTT Broker Port, e.g. 1883]
    * mqttUsr=[MQTT Broker Username - can be empty]
    * mqttPwd=[MQTT Broker Password - can be empty]
 - values.txt file can be empty
 - Map the GoFarmTech/src/web/build folder to the %your_project_dir%/data/web folder (just symlink or copy)
 - Upload data folder to the device: 'Upload File System Image' Task in PlatformIO. This step required only at the beginning
 - Define SETTINGS_PIN & SETTINGS_LED values in main.cpp (**before** any GoFarmTech library include file) for Maintencance Knob & Led definition
 - Include library files and define the device main class with the device logic
 - See example for the reference
 - Compile and upload your project to the device
 - Open the serial port console and check the runtime logs
 - Press for few secconds the Maintenace Knob attached to the SETTINGS_PIN, the SETTINGS_LED led should flare up
 - Open the WiFi settings on your smartphone and connect to the %deviceId% WiFi network (e.g. IoTMyConditionTimer)
 - Wait for few seconds - the captive portal application will be opened
 - Press the menu button and go to the Global Settings, specify the basic network configuration
 - Press Apply button: device will be configured with your settings, the captive portal will be closed automatically, the SETTINGS_LED will be flare down
 - Use the same method for the sensors configuration. Press Maintenace Knob at end of the configuration process, the Maintenance HotSpot will be clossed automatically
 - Add GoFarmTech entry to the lib_deps key in platformio.ini file

## Known limitation
The Framework was tested on esp8266 platform only. Please open the issue in case of some questions or trobles, I'll answer as soon as I can. Feel free to modify this code, apply logs or implement our own sensors - any help is welcomed.

## Roadmap
 - esp32 support
 - Device password protection (deviceUsr & devicePwd)
 - Encrypt username & passwords in settings.txt file
 - Provide support for other sensors
 - LoRaWAN support (?)


## License
This code is released under the MIT License.