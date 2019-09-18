//  SettingsManager.cpp - GoFarmTech Framework Source Code
//  Author: bratello

#include "SettingsManager.h"
#include "Settings.h"
#include "NetworkManager.h"
#include <DNSServer.h>
#if defined(ESP8266)
    #include <ESP8266WebServer.h>
    #include <detail/mimetable.h>
#else
    #include <WebServer.h>
#endif
#include <TimeLib.h>

#include <vector>
#include <utility>
#include <functional>

#define SETTINGS_SKIP_TIME 250

class AccessPoint : public Runnable, public Loggable {
    LOGGABLE(AccessPoint)
protected:
    typedef std::function<void(void)> getFileCallback_t;
    typedef std::function<String(Stream&)> getFileModifier_t;

    const byte DNS_PORT = 53;
    DNSServer dnsServer;
#ifdef ESP8266
    ESP8266WebServer webServer;
#else
    WebServer webServer;
#endif
    Descriptable*     _deviceDescriptor;
    MQTTClientTransmitter*     _client;

    virtual	void	doLoop();
    getFileCallback_t   handleGetFile(const String& fileName, const String& contentType, const getFileModifier_t& modifier = getFileModifier_t());

    void    onSaveDeviceSettings();
    void    onGetDeviceSettingsAndDescription();
    void    onSaveDeviceValue();
    void    onGetDeviceValue();
    void    onSetCurrentTime();

    void    doJsonResponse(const String& data = "{\"success\": true}");
public:
    typedef getFileCallback_t onCloseAPCallback_t;
protected:
    onCloseAPCallback_t     _closeAPCallback;
    uint8_t                 _leadPin;
public:
	virtual	void	setup();
public:
    time_t	loop(time_t time = 0);

    AccessPoint(Descriptable* device, MQTTClientTransmitter* client, uint8_t leadPin, const onCloseAPCallback_t& cb);
    virtual ~AccessPoint();
};


SettingsManager::SettingsManager(uint8_t leadPin, uint8_t switchPin) : _modeLeadPin(leadPin), 
                                                                        _switchPin(switchPin), 
                                                                        _apMode(false),
                                                                        _deviceDescriptor(NULL),
                                                                        _client(NULL)
{
    this->_skipTime = SETTINGS_SKIP_TIME;
}

void SettingsManager::setDeviceDescriptor(Descriptable* device) {
    _deviceDescriptor = device;
}

void SettingsManager::setClient(MQTTClientTransmitter* client) {
    _client = client;
}

void	SettingsManager::setup() {
    pinMode(_switchPin, INPUT);
    pinMode(_modeLeadPin, OUTPUT);
    digitalWrite(_modeLeadPin, HIGH);
    digitalWrite(_switchPin, HIGH);
}

void	SettingsManager::doLoop() {
    if(!digitalRead(_switchPin)) {
        delay(200);
        if(digitalRead(_switchPin)) {
            return;
        }
        _apMode = !_apMode;
    }
    if(_apMode) {
        if(!_accessPoint) {
            _skipTime = 0;
            _accessPoint = std::move(RunnablePtr(new AccessPoint(_deviceDescriptor, _client, _modeLeadPin, [this] () {
                _skipTime = SETTINGS_SKIP_TIME;
                _apMode = false;
            })));
            _accessPoint->setup();
            while(!digitalRead(_switchPin)) {
                delay(100);
            }
        }
        _accessPoint->loop();
    } else if(_accessPoint) {
        LOGGER(info("Close AccessPoint"));
        _accessPoint.reset(nullptr);
    }
}

AccessPoint::AccessPoint(Descriptable* descriptor, MQTTClientTransmitter* client, uint8_t leadPin, const onCloseAPCallback_t& cb) : webServer(80), _deviceDescriptor(descriptor), _client(client), _closeAPCallback(cb), _leadPin(leadPin) {
    LOGGER(info("ctor()"));
    _skipTime = 0;
    digitalWrite(_leadPin, LOW);
}

AccessPoint::~AccessPoint() {
    LOGGER(info("~dtor() start"));
    digitalWrite(_leadPin, HIGH);
    webServer.stop();
    dnsServer.stop();
    netManager.disconnectSetup();
    LOGGER(info("~dtor() end"));
}

void AccessPoint::doLoop() {}

void AccessPoint::setup() {
    LOGGER(info("AccessPoint Setup"));
    netManager.connectSetup();

    LOGGER(info("DNS Setup"));
    // if DNSServer is started with "*" for domain name, it will reply with
    // provided IP to all DNS request
    dnsServer.start(DNS_PORT, "*", netManager.getSetupIP());

    LOGGER(info("Index File Setup"));
    // replay to all requests with same HTML
    webServer.onNotFound(handleGetFile("/web/index.html.gz", "text/html"));

    LOGGER(info("MimeTable Setup"));
    std::map<String, String> mimeTypes; 
    for(int i = mime::type::html; i < mime::type::none; i++) {
        auto entity = mime::mimeTable[i];
        LOGGER(info("MimeType: ")(entity.endsWith)(", ")(entity.mimeType));
        mimeTypes[entity.endsWith] = entity.mimeType;
    }
        
    
    LOGGER(info("File Table Setup)"));
    String webLocation = "/web";
    String gzExt = ".gz";
    for(auto& file : Settings::instance()->readDir(webLocation)) {
        auto item = file.substring(webLocation.length());
        if(item.endsWith(gzExt)){
            item = item.substring(0, item.length() - gzExt.length());
        }
        auto ext = item.substring(item.lastIndexOf("."));
        String mimeTipe = "application/octet-stream";
        if(mimeTypes.find(ext) != mimeTypes.end()) {
            mimeTipe = mimeTypes[ext];
        }
        LOGGER(info("Init handleGetFile: ")(file)(", ")(item)(", ")(mimeTipe));
        webServer.on(item, HTTP_GET, handleGetFile(file, mimeTipe));
    }
    webServer.on("/api/deviceSettings", HTTP_POST, std::bind(&AccessPoint::onSaveDeviceSettings, this));
    webServer.on("/api/value", HTTP_POST, std::bind(&AccessPoint::onSaveDeviceValue, this));
    webServer.on("/api/value", HTTP_GET, std::bind(&AccessPoint::onGetDeviceValue, this));
    webServer.on("/api/currentTime", HTTP_POST, std::bind(&AccessPoint::onSetCurrentTime, this));
    webServer.on("/api/deviceSettingsAndDescription", HTTP_GET, std::bind(&AccessPoint::onGetDeviceSettingsAndDescription, this));
    webServer.begin();
}

AccessPoint::getFileCallback_t   AccessPoint::handleGetFile(const String& fileName, const String& contentType, const AccessPoint::getFileModifier_t& modifier) {
    return [this, fileName, contentType, modifier] () {
        LOGGER(info("handleGetFile: ")(fileName)(", uri: ")(webServer.uri()));
        if(String("text/html") == contentType) {
            webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
            webServer.sendHeader("Pragma", "no-cache");
            webServer.sendHeader("Expires", "-1");
        }
        auto f = Settings::instance()->readFile(fileName);
        if(f) {
            LOGGER(info("Read File: ") + fileName);
            if(modifier) {
                webServer.send(200, contentType, modifier(f));
            } else {
                webServer.streamFile(f, contentType);
            }
            f.close();
        } else {
            auto msg = String("File not found: ") + fileName;
            webServer.send(404, "text/plain", msg);
            LOGGER(error(1, msg));
        }
    };
}

void AccessPoint::onGetDeviceSettingsAndDescription() {
    LOGGER(info("onGetDeviceSettingsAndDescription"));
    String content = "{\"DeviceSettings\":" + Settings::instance()->toJSON() + ",";
    content += "\"DeviceDescription\":" + _deviceDescriptor->getDescription().toJSON() + "}";
    doJsonResponse(content);
}

void AccessPoint::onSaveDeviceSettings() {
    LOGGER(info("onSaveDeviceSettings"));
    auto settings = Settings::instance();
    auto onChangeEnabled = settings->enableOnChanged(false);

    auto args = webServer.args();
    for(int i = 0; i < args; i++) {
        auto propName = webServer.argName(i);
        auto propVal = webServer.arg(i);
        LOGGER(info("onSaveDeviceSettings: ")("propName = ")(propName)(", propVal = ")(propVal));
        if(settings->hasProperty(propName)) {
            settings->write(propName, propVal);
        } else {
            LOGGER(info("System setting ")(propVal)(" not exists"));
        }
    }

    //Return back the original callback before the last 
    settings->enableOnChanged(onChangeEnabled);
    settings->mqttPort = (int)settings->mqttPort;
    doJsonResponse();
    _closeAPCallback();
}

void AccessPoint::onSaveDeviceValue() {
    LOGGER(info("onSaveDeviceSettings"));
    auto args = webServer.args();
    for(int i = 0; i < args; i++) {
        auto propName = webServer.argName(i);
        auto propVal = webServer.arg(i);
        LOGGER(info("onSaveDeviceSettings: ")("propName = ")(propName)(", propVal = ")(propVal));
        if(!_client->transmit(propName, propVal)) {
            LOGGER(info("Value not exists: ")(propName));
        } else {
            LOGGER(info("Value ")(propName)(" transmitted"));
        }
    }
    doJsonResponse();
}

void AccessPoint::onSetCurrentTime() {
    LOGGER(info("onSetCurrentTime"));
    auto timeStr = webServer.arg("currentTime");
    setTime(from_string<time_t>(timeStr));
    LOGGER(info("System time changed: ")(timeStr));
    doJsonResponse();
}

void AccessPoint::onGetDeviceValue() {
    String valueName = webServer.arg("name");
    LOGGER(info("onGetDeviceValue: ")(valueName));
    String ret;
    Description::setDescriptionTrapper([this, valueName, &ret] (const Description& parent, const Description& child) {
        if(ret.length()) {
            return;
        }
        auto path = parent.getName() + "/" + child.getName();
        LOGGER(info("Value path: ")(path));
        auto id = path.lastIndexOf(valueName);
        if(id != -1 && valueName.length() == path.length() - id) {
            ret = child.getValue();
            LOGGER(info("Trapped value: ")(ret))
        }
    });
    _deviceDescriptor->getDescription();
    Description::setDescriptionTrapper();
    if(!ret.length()) {
        ret = "null";
    }
    doJsonResponse(String("{\"value\":") + ret + "}");
}

void AccessPoint::doJsonResponse(const String& data) {
    webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    webServer.sendHeader("Pragma", "no-cache");
    webServer.sendHeader("Expires", "-1");
    webServer.send(200, "application/json", data);
}

time_t AccessPoint::loop(time_t time) {
    dnsServer.processNextRequest();
    webServer.handleClient();
    return _skipTime;
}