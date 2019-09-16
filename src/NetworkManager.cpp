//  NetworkManager.cpp - GoFarmTech Framework Source Code
//  Author: bratello

#include 	"NetworkManager.h"
#if defined(ESP8266)
	#include 	<ESP8266WiFi.h>
#else
    #include <WiFi.h>
#endif
#include 	"Settings.h"

extern "C" {
  #include "user_interface.h"
}


static NetworkManager	instance;
NetworkManager& netManager = instance;
static IPAddress zeroIP = IPAddress(0, 0, 0, 0);

bool    NetworkManager::connect() {
    String ssid = Settings::instance()->ssid, pwd = Settings::instance()->wifiPwd;
	LOGGER(info("Connecting to ") + ssid)
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid.c_str(), (pwd.length() > 0 ? pwd.c_str() : NULL));

	int attempts = 0;
	while (!isConnected()) {
		delay(500);
		if(attempts++ > 10) {
			LOGGER(warn(1, "Unable to connect to wifi '") + ssid + "', check network settings")
			return false;
		}
	}

	if(isConnected()) {
		LOGGER(info("WiFi connected, IP address: ") + WiFi.localIP())
		delay(500);
	}
    return true;
}

bool    NetworkManager::disconnect() {
    return WiFi.disconnect(false);
}

bool    NetworkManager::isConnected() {
    return (WiFi.getMode() & WIFI_STA) && WiFi.status() == WL_CONNECTED;
}

bool    NetworkManager::lightSleep(unsigned long t) {
	if(!isConnected()) {
		return true;
	}
	wifi_set_sleep_type(LIGHT_SLEEP_T);
	delay(t);
	return wifi_set_sleep_type(NONE_SLEEP_T);
}

bool    NetworkManager::connectSetup() {
	disconnect();
	IPAddress apIP = IPAddress(192, 168, 1, 1);
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    String ssid = Settings::instance()->deviceId;
	String devicePwd = Settings::instance()->devicePwd;
	int attempts = 0;
	while(!WiFi.softAP(ssid.c_str(), (devicePwd.length() > 0 ? devicePwd.c_str() : NULL))) {
		delay(500);
		if(attempts++ > 10) {
			LOGGER(error(1, "Unable to establish softAP mode with IP address: ") + apIP + ", ssid: " + ssid);
        	return false;
		}
	}
    LOGGER(info("Setup network established: ")(ssid)(". SoftAP IP: ")(WiFi.softAPIP()));
	return true;
}

bool    NetworkManager::disconnectSetup() {
	if(isSetupConnected()) {
		return connect();
	}
	return true;
}

bool    NetworkManager::isSetupConnected() {
	return (WiFi.getMode() & WIFI_AP) && WiFi.softAPIP() != zeroIP;
}

IPAddress   NetworkManager::getSetupIP() {
	return WiFi.softAPIP();
}

bool        NetworkManager::pingHost(const String& host, int port) {
	WiFiClient client;

	int result = client.connect(host, port);
	if (!result) {
		LOGGER(info("PingHost connection failed: ")(result));
		return false;
	}

	LOGGER(info("PingHost connection success: ")(result));
	client.stop();
	return true;
}