# GoFarmTech Arduino based Framework for developing agriculture/industry process automation device.

This library provides a full feautured framework for development of serverless multifunction controller which able to controll process in unstable network connection environment. In some of the industry processes the cost of network failure is very high - e.g. watering process in agriculture. So the device should be able to handle process independently from server, but allow to communicate with the MQTT server when it's possible. The main keys of the framework:

 - Set of the sensor and relay classes are ready to use
 - Simple sensor registration (see example)
 - Framework examines every sensor and posts the value change to the MQTT server (if possible right now)
 - Sensor supports the value change event for device logic implementation
 - Sensor supports min/max value limits and events when the value reaches the limits
 - Built in multitask timer with various task execution strategies for building correct device logic
 - Device can be configured through the Web UI independenty from the server (Initial network setup, sensor limits, value examination frequency, etc)
 - Ready to use inegration with ioBroker, which allows to controll and configure the device from the server: [ioBroker.gofarmtech](https://github.com/bratello/ioBroker.gofarmtech)
 - Logging system for maintenance and diagnostics

## WebUI Screenshots
 - ![Timer Settings](screenshots/timerSettingsUI.jpg)
 - ![Global Settings](screenshots/globalSettingsUI.jpg)

## Dependencies and Requirements
 1. [Time](http://playground.arduino.cc/code/time): the device time will be updated from Web UI or from ioBroker automatically
 2. [DHT sensor library](https://github.com/adafruit/DHT-sensor-library)
 3. [PubSubClient](https://github.com/knolleary/pubsubclient): please install PubSubClient to the local lib folder and modify the PubSubClient::domain member's type from **const char\*** to the **String** (well known boggus pointer issue)
 4. [Arduino_JSON](http://github.com/arduino-libraries/Arduino_JSON)
 5. SPIFFS should be supported and available, in about 400K FS space is required.

## Build steps
 1. Install all dependencies and requirements
 2. Specify the data folder (data_dir in PlatformIO)
 3. Create settings.txt & values.txt in data folder
 4. Specify the following keys in settings.txt (without brackets or quotes, just a value after the equal sign)
    * deviceId=[Your device ID e.g.IoTMyConditionTimer - this ID will be used as Maintenance WiFi HotSpot name and as the MQTT channel ID]
    * deviceUsr=[Empty for now, for future use]
    * devicePwd=[Empty for now, for future use]
    * ssid=[WiFi network name for ioBroker connection]
    * wifiPwd=[WiFi password]
    * mqttHost=[MQTT Broker IP e.g. 192.168.0.120]
    * mqttPort=[MQTT Broker Port, e.g. 1883]
    * mqttUsr=[MQTT Broker Username - can be empty]
    * mqttPwd=[MQTT Broker Password - can be empty]
 5. values.txt file can be empty
 6. Map the GoFarmTech/src/web/build folder to the %your_project_dir%/data/web folder (just symlink or copy)
 7. Upload data folder to the device: 'Upload File System Image' Task in PlatformIO. This step required only at the beginning
 8. Define SETTINGS_PIN & SETTINGS_LED values in main.cpp (**before** any GoFarmTech library include file) for Maintencance Knob & Led definition
 9. Include library files and define the device main class with the device logic
 10. See [example](examples/main.cpp) for the reference
 11. Compile and upload your project to the device
 12. Open the serial port console and check the runtime logs
 13. Press for few secconds the Maintenace Knob attached to the SETTINGS_PIN, the SETTINGS_LED led should flare up
 14. Open the WiFi settings on your smartphone and connect to the %deviceId% WiFi network (e.g. IoTMyConditionTimer)
 15. Wait for few seconds - the captive portal application will be opened
 16. Press the menu button and go to the Global Settings, specify the basic network configuration
 17. Press Apply button: device will be configured with your settings, the captive portal will be closed automatically, the SETTINGS_LED will be flare down
 18. Use the same method for the sensors configuration. Press Maintenace Knob at end of the configuration process, the Maintenance HotSpot will be clossed automatically
 19. Add GoFarmTech entry to the lib_deps key in platformio.ini file

## Known limitation
The Framework was tested on esp8266 platform only. Please open the issue in case of some questions or troubles, I'll answer as soon as I can. Feel free to modify this code, apply logs or implement our own sensors - any help is welcomed.

## Roadmap
 - esp32 support
 - Device password protection (deviceUsr & devicePwd)
 - Encrypt username & passwords in settings.txt file
 - Provide support for other sensors
 - LoRaWAN support (?)


## License
This code is released under the MIT License.
