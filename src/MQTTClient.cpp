//  MQTTClient.cpp - GoFarmTech Framework Source Code
//  Author: bratello

#include	<PubSubClient.h>
#include	<list>
#include	<memory>

#include	"MQTTClient.h"
#include	"TimerTask.h"
#include	"NetworkManager.h"
#include	"Settings.h"


MQTTClient::MQTTClient() : Runnable(), _deviceDescriptor(NULL) { }

class MQTTChildClientBase : public MQTTClient {
protected:
	typedef std::unique_ptr<MQTTClient>	MQTTClientPtr;
	typedef	std::list<MQTTClientPtr>	MQTTClientList_t;

	String 				_topic;
	MQTTClientList_t	_children;
protected:
	String			getFullTopicName(const String& topic);
	virtual void 	doSubscribe(const String& topic, const payload_callback_t& cb);
	virtual void	doPublish(const String& topic, const String& value);
public:
	MQTTChildClientBase(const String& topic = "") : MQTTClient(), _topic(topic) {}
	virtual MQTTClient* getChildClient(const String& topic);
	virtual String getClientPayload(bool bClear = false);
	virtual	payload_callback_t	findTopicSubscriber(const String& topic);

public:
	virtual bool transmit(const String& topic, const String& value);
};

String MQTTChildClientBase::getClientPayload(bool bClear) {
	String payload = accumulate_chain<String>(_topicValues, [this] (const String& init, const topic_value_map_t::value_type& it) {
		return init + (init.length() > 0 ? "," : "") + "\"" + it.first + "\":" + it.second;
	});
	if(bClear && !_topicValues.empty()) {
		_topicValues.clear();
	}
	for(auto& child : _children) {
		auto childPayload = child->getClientPayload(bClear);
		if(childPayload.length() > 0) {
			payload += (payload.length() > 0 ? "," : "") + childPayload;
		}
	}
	return payload;
}
void MQTTChildClientBase::doSubscribe(const String& topic, const payload_callback_t& callback) {
	auto topicName = getFullTopicName(topic);
	_topicSubscribers[topicName] = callback;
	//LOGGER(info("Subscribed: ") + topicName);
}

void MQTTChildClientBase::doPublish(const String& topic, const String& value) {
	auto topicName = getFullTopicName(topic);
	_topicValues[topicName] = value;
	//LOGGER(info("Published: ") + topicName);
}

payload_callback_t	MQTTChildClientBase::findTopicSubscriber(const String& topic) {
	auto it = _topicSubscribers.find(topic);
	if(it != _topicSubscribers.end()) {
		return it->second;
	}
	for(auto& child: _children) {
		auto subscriber = child->findTopicSubscriber(topic);
		if(subscriber) {
			return subscriber;
		}
	}
	return payload_callback_t();
}

String	MQTTChildClientBase::getFullTopicName(const String& topic) {
	return (_topic.length() > 0 ? (topic.length() > 0 ? _topic + "/" + topic : _topic) : topic);
}

class MQTTChildClient : public MQTTChildClientBase {
protected:
	virtual		void	doLoop() {}
public:
	MQTTChildClient(const String& topic) : MQTTChildClientBase(topic) {}
	virtual		void	setup()	{}
};

MQTTClient* MQTTChildClientBase::getChildClient(const String& topic) {
	auto client = new MQTTChildClient(( _topic.length() > 0 ? _topic + "/" : "") + topic);
	_children.push_back(std::move(std::unique_ptr<MQTTClient>(client)));
	return client;
}

bool MQTTChildClientBase::transmit(const String& topic, const String& value) {
	auto subscriber = findTopicSubscriber(topic);
	if(subscriber) {
		subscriber(value);
		return true;
	}
	return false;
}

class MQTTClientImpl : public MQTTChildClientBase {
protected:
	PubSubClient	_mqttClient;
	int 			_connectAttempts = 0;
	time_t 			_lastConnectionAttempt = 0;
	const 	int 	_maxConnectAttempts = 3;
	const 	int 	_connectRetryTimeout = 30;
	bool			_firstPublish;

	String getTopicPath(const String& topic);
public:
	MQTTClientImpl() : MQTTChildClientBase(), _connectAttempts(), _lastConnectionAttempt(), _firstPublish(true) {}

	void	setClient(Client& client) {
		_mqttClient.setClient(client);
	}

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
};

MQTTClient* MQTTClient::instance(Client& client) {
	static MQTTClientImpl obj;
	obj.setClient(client);
	return &obj;
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
		doSetServer();
		if(connect())
			subscribe();
	});
	if(connect())
		subscribe();
}

void MQTTClientImpl::doLoop() {
	if(!netManager.isConnected()) {
		DOIT_ONCE(LOGGER(info("Network disconnected")); netManager.disconnect(); netManager.connect(), 300)
		return;
	}
	if(!_mqttClient.connected()) {
		DOIT_ONCE(LOGGER(info("MQTTClient disconnected, reconnect")), 10)
		if(connect())
			subscribe();
	}
	if(_mqttClient.connected()) {
		_mqttClient.loop();
		flushDeviceDescription();
		flush();
		_firstPublish = false;
	} else {
		DOIT_ONCE(LOGGER(info("MQTTClient still disconnected")), 10)
	}
}

String MQTTClientImpl::getTopicPath(const String& topic) {
	String deviceId = Settings::instance()->deviceId;
	return String("GoFarmTechClient/") + deviceId + (topic.length() ? "/" + topic : String(""));
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
		payload = "{" + payload + "}";
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
	auto topic = getTopicPath("");
	auto json = _deviceDescriptor->getDescription().toJSON();
	if(!mqttPublish(topic, json, "Device description")) {
		LOGGER(warn(5, "Device descriptor publishing failed"))
	}
	_mqttClient.loop();
	delay(500);
}
