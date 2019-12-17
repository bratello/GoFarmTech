//  MQTTClient.cpp - GoFarmTech Framework Source Code
//  Author: bratello

#include	<PubSubClient.h>
#include	<list>
#include	<memory>

#include	"MQTTClient.h"
#include	"TimerTask.h"
#include	"NetworkManager.h"
#include	"Settings.h"

class MQTTClientImpl : public MQTTClient {
protected:
	typedef		std::map<String, String>				topic_value_map_t;
	typedef		std::map<String, payload_callback_t>	topic_subscriber_map_t;
	topic_value_map_t									_topicValues;
	topic_subscriber_map_t								_topicSubscribers;
	Descriptable*										_deviceDescriptor;

protected:
	PubSubClient	_mqttClient;
	int 			_connectAttempts = 0;
	time_t 			_lastConnectionAttempt = 0;
	const 	int 	_maxConnectAttempts = 3;
	const 	int 	_connectRetryTimeout = 30;
	bool			_firstPublish = false;

	String getTopicPath(const String& topic);
	payload_callback_t	findTopicSubscriber(const String& topic);
	String getClientPayload(bool bClear);
public:
	MQTTClientImpl() : _deviceDescriptor(NULL) {}

	void setClient(Client& client) {
		_mqttClient.setClient(client);
	}

protected:
	virtual void 	doSubscribe(const String& topic, const payload_callback_t& callback);
	virtual void	doPublish(const String& topic, const String& value);
public:
	virtual MQTTClient* 		getChildClient(const String& topic);
	virtual void setDeviceDescriptor(Descriptable* deviceDescriptor);


//Runnable implementation
protected:
	virtual		void	doLoop();
	bool				mqttPublish(const String& topic, const String& payload, const String& help);
public:
	virtual		void	setup();

public:
	bool		connect();
	bool		subscribe();
	void		flush();
	void		flushDeviceDescription();

	virtual bool transmit(const String& topic, const String& value);
};

void	MQTTClientImpl::doSubscribe(const String& topic, const payload_callback_t& callback) {
	auto topicName = topic;
	_topicSubscribers[topicName] = callback;
	//LOGGER(info("Subscribed: ") + topicName);
}

void	MQTTClientImpl::doPublish(const String& topic, const String& value) {
	auto topicName = topic;
	_topicValues[topicName] = value;
	//LOGGER(info("Published: ") + topicName);
}

void MQTTClientImpl::setDeviceDescriptor(Descriptable* deviceDescriptor) {
	_deviceDescriptor = deviceDescriptor;
}

bool MQTTClientImpl::connect() {
	if(_connectAttempts >= _maxConnectAttempts) {
		time_t nowValue = now();
		if(_lastConnectionAttempt == 0) {
			_lastConnectionAttempt = nowValue;
			return false;
		} else if(_lastConnectionAttempt + _connectRetryTimeout > nowValue) {
			return false;
		} else {
			_connectAttempts = 0;
			_lastConnectionAttempt = 0;
			LOGGER(info("Connection retry after ") + _connectRetryTimeout + " seconds")
		}
	}
	_connectAttempts++;
	LOGGER(info("Connecting..."))
	String clientId = Settings::instance()->deviceId;
	bool	result = false;
	auto settings = Settings::instance();
	String mqttUsr = settings->mqttUsr;
	String mqttPwd = settings->mqttPwd;
	if(mqttUsr.length() && mqttPwd.length())
		result = _mqttClient.connect(clientId.c_str(), mqttUsr.c_str(), mqttPwd.c_str());
	else
		result = _mqttClient.connect(clientId.c_str());
	delay(500);
	if(result && _mqttClient.connected()) {
		LOGGER(info("Client connected, ClientID: ")(clientId)
			(", MQTT User: ")(mqttUsr.length() ? mqttUsr : "<empty>")
			(", MQTT PWD: ")(mqttPwd.length() ? "*******" : "<empty>")
			(", HOST: ")((String)settings->mqttHost)
			(", PORT: ")((int)settings->mqttPort)
		)
		_connectAttempts = 0;
		_lastConnectionAttempt = 0;
	} else {
		LOGGER(
			info("Unable to connect to MQTT server, ClientID: ")(clientId)
			(", MQTT User: ")(mqttUsr.length() ? mqttUsr : "<empty>")
			(", MQTT PWD: ")(mqttPwd.length() ? "*******" : "<empty>")
			(", HOST: ")((String)settings->mqttHost)
			(", PORT: ")((int)settings->mqttPort)
			(", result: ")(result)
		)
		netManager.pingHost(settings->mqttHost, settings->mqttPort);
	}
	return result;
}

void MQTTClientImpl::setup() {
	if(_mqttClient.connected()) {
		LOGGER(info("Disconnecting..."))
		_mqttClient.disconnect();
	}
	auto doSetServer = [this] () {
		auto settings = Settings::instance();
		String host = settings->mqttHost;
		int port = settings->mqttPort;
		_mqttClient.setServer(host.c_str(), port);
		LOGGER(info("Set MQTT Server, host: ")(host)(", port: ")(port))
	};

	LOGGER(info("Client setup..."))
	doSetServer();
	_mqttClient.setCallback([this](char* topic, uint8_t* payload, unsigned int length) {
		auto payloadStr = from_mqtt<String>(payload, length);
		split_string(payloadStr, "\r\n", [this] (const String& item) {
			auto pos = item.indexOf("=");
			if(pos == -1) {
				LOGGER(error(1, "Wrong mqtt message entry: ") + item)
				return;
			}
			auto topic = item.substring(0, pos);
			topic.trim();
			auto value = item.substring(pos + 1);
			value.trim();
			auto subscriber = findTopicSubscriber(topic);
			if(!subscriber) {
				LOGGER(error(2, "Wrong topic: ") + topic)
				return;
			}
			LOGGER(info("Call subscriber '") + topic + "' with value: " + value)
			subscriber(value);
		});
	});
	Settings::instance()->onChanged([this, doSetServer] () {
		LOGGER(info("Settings changed"))
		_connectAttempts = 0;
		_firstPublish = true;
		doSetServer();
		if(connect())
			subscribe();
	});
	if(connect())
		subscribe();
}

void MQTTClientImpl::doLoop() {
	if(!netManager.isConnected()) {
		return;
	}
	if(!_mqttClient.connected()) {
		DOIT_ONCE(LOGGER(info("MQTTClient disconnected, reconnect"));if(connect()) subscribe(), 120)
	}
	if(_mqttClient.connected()) {
		_mqttClient.loop();
		flushDeviceDescription();
		flush();
		_firstPublish = false;
	} else {
		DOIT_ONCE(LOGGER(info("MQTTClient still disconnected")), 120)
	}
}

bool MQTTClientImpl::subscribe() {
	if(_mqttClient.connected()) {
		auto topic = getTopicPath("subscribe");
		LOGGER(info("Subscribed to: ") + topic)
		return _mqttClient.subscribe(topic.c_str());
	}
	return false;
}

bool MQTTClientImpl::mqttPublish(const String& topic, const String& payload, const String& help) {
	static int failureCount = 0;
	bool ret = false;
	if(MQTT_MAX_PACKET_SIZE < MQTT_MAX_HEADER_SIZE + 2 + topic.length() + payload.length()) {
		ret = _mqttClient.beginPublish(topic.c_str(), payload.length(), false);
		if(ret) {
			_mqttClient.print(payload);
			_mqttClient.endPublish();
		}
	} else {
		ret = _mqttClient.publish(topic.c_str(), payload.c_str());
	}
	if(ret) {
		failureCount = 0;
		LOGGER(info(help) + " published: " + payload.length() + " bytes by topic '" + topic + "'")
	} else {
		failureCount++;
		auto len = payload.length() + topic.length();
		if(MQTT_MAX_PACKET_SIZE <= len) {
			LOGGER(error(3, help) + " is not published to topic '" + topic + "'. MQTT_MAX_PACKET_SIZE is too small, increase it at list " + len + " bytes")
		} else {
			LOGGER(error(4, help) + 
			" is not published to topic '" + 
			topic + "', data size: " + 
			len + " bytes" + 
			", data: " + payload)
		}
		if(failureCount > 1000) {
			LOGGER(info("Restarting the controller..."));
			ESP.restart();
		}
	}

	return ret;
}

void MQTTClientImpl::flush() {
	if(!_mqttClient.connected()) {
		return;
	}
	if(_firstPublish) {
		return;
	}
	auto payload = getClientPayload(true);
	if(payload.length()) {
		auto topic = getTopicPath("publish");
		if(mqttPublish(topic, payload, "Value")) {
			_topicValues.clear();
		}
	}
}

void MQTTClientImpl::flushDeviceDescription() {
	if(!_firstPublish || !_deviceDescriptor || !_mqttClient.connected()) {
		return;
	}
	if(!Settings::instance()->xRegDevice) {
		//Device already registered
		return;
	}
	auto topic = getTopicPath("");
	auto json = _deviceDescriptor->getDescription().toJSON();
	if(!mqttPublish(topic, json, "Device description")) {
		LOGGER(warn(5, "Device descriptor publishing failed"))
	} else {
		Settings::instance()->xRegDevice = false;
		LOGGER(info("Device Description published"))
	}
	_mqttClient.loop();
	delay(500);
}

String MQTTClientImpl::getTopicPath(const String& topic) {
	String deviceId = Settings::instance()->deviceId;
	return String("GoFarmTechClient/") + deviceId + (topic.length() ? "/" + topic : String(""));
}

payload_callback_t	MQTTClientImpl::findTopicSubscriber(const String& topic) {
	auto it = _topicSubscribers.find(topic);
	if(it != _topicSubscribers.end()) {
		return it->second;
	}
	return payload_callback_t();
}

String MQTTClientImpl::getClientPayload(bool bClear) {
	String payload = accumulate_chain<String>(_topicValues, [this] (const String& init, const topic_value_map_t::value_type& it) {
		return init + (init.length() > 0 ? "," : "") + "\"" + it.first + "\":" + it.second;
	});
	if(bClear && !_topicValues.empty()) {
		_topicValues.clear();
	}
	if(payload.length() > 0) {
		payload = "{" + payload + "}";
	}
	return payload;
}

bool MQTTClientImpl::transmit(const String& topic, const String& value) {
	auto subscriber = findTopicSubscriber(topic);
	if(subscriber) {
		subscriber(value);
		return true;
	}
	return false;
}

MQTTClient::MQTTClient() : Runnable() { }

MQTTClient* MQTTClient::instance(Client& client) {
	static MQTTClientImpl obj;
	obj.setClient(client);
	return &obj;
}

class MQTTChildClient : public MQTTClient {
protected:
	String			_name;
	MQTTClient*		_parent;
	String	getFullTopicName(const String& topic);
public:
	MQTTChildClient(const String& name, MQTTClient* parent) : _name(name), _parent(parent) {}

protected:
	virtual void 	doSubscribe(const String& topic, const payload_callback_t& callback);
	virtual void	doPublish(const String& topic, const String& value);
public:
	virtual MQTTClient* getChildClient(const String& name);
	virtual void setDeviceDescriptor(Descriptable* deviceDescriptor);


//Runnable implementation
protected:
	virtual		void	doLoop();
	bool				mqttPublish(const String& topic, const String& payload, const String& help);
public:
	virtual		void	setup();
	virtual bool transmit(const String& topic, const String& value);
};

String	MQTTChildClient::getFullTopicName(const String& topic) {
	return (_name.length() > 0 ? (topic.length() > 0 ? _name + "/" + topic : _name) : topic);
}

void MQTTChildClient::doSubscribe(const String& topic, const payload_callback_t& callback) {
	auto topicName = getFullTopicName(topic);
	_parent->doSubscribe(topicName, callback);
	//LOGGER(info("Subscribed: ") + topicName);
}

void MQTTChildClient::doPublish(const String& topic, const String& value) {
	auto topicName = getFullTopicName(topic);
	_parent->doPublish(topicName, value);
	//LOGGER(info("Published: ") + topicName);
}

MQTTClient* MQTTChildClient::getChildClient(const String& name) {
	auto client = new MQTTChildClient(( _name.length() > 0 ? _name + "/" : "") + name, this);
	return client;
}

void MQTTChildClient::setDeviceDescriptor(Descriptable* deviceDescriptor) {
	_parent->setDeviceDescriptor(deviceDescriptor);
}

void MQTTChildClient::doLoop() {}
void MQTTChildClient::setup() {}
bool MQTTChildClient::transmit(const String& topic, const String& value) {
	return _parent->transmit(topic, value);
}

MQTTClient* MQTTClientImpl::getChildClient(const String& name) {
	auto client = new MQTTChildClient(name, this);
	return client;
}