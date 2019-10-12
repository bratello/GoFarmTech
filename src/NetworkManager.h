//  NetworkManager.h - GoFarmTech Framework Source Code
//  Author: bratello

#ifndef MQTT_WIFI_MANAGER_H
#define MQTT_WIFI_MANAGER_H
#include	<Arduino.h>
#include	"Loggable.h"

enum class NetworkConnectionStatus {
    DISCONNECTED,
    CONNECTING,
    CONNECTED
};

class NetworkManager : public Loggable {
    LOGGABLE(NetworkManager)
public:
    bool        connect();
    bool        disconnect();
    bool        isConnected();
    bool        lightSleep(unsigned long time);

    bool        connectSetup();
    bool        disconnectSetup();
    bool        isSetupConnected();
    IPAddress   getSetupIP();
    bool        pingHost(const String& host, int port);
};

extern NetworkManager& netManager;

#endif