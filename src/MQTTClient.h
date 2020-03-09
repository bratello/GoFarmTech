//  MQTTClient.h - GoFarmTech Framework Source Code
//  Author: bratello

#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H
#include	<Arduino.h>
#include	<Client.h>
#include	<functional>
#include	<map>
#include	<memory>
#include	"Loggable.h"
#include	"Runnable.h"
#include 	"Descriptable.h"
#include	"Convertors.h"


class MQTTClientTransmitter {
public:
	virtual bool transmit(const String& topic, const String& value) = 0;
};

typedef		std::function<void (const String&)>	payload_callback_t;

template<typename T>
payload_callback_t	make_callback(const std::function<void (const T&)>& callback);

class MQTTClient : public Loggable, public Runnable, public MQTTClientTransmitter {
	LOGGABLE(MQTTClient)
protected:
	MQTTClient();
	virtual void 	doSubscribe(const String& topic, const payload_callback_t& callback) = 0;
	virtual void	doPublish(const String& topic, const String& value) = 0;
public:
	virtual MQTTClient* 		getChildClient(const String& name) = 0;
	virtual void setDeviceDescriptor(Descriptable* deviceDescriptor) = 0;
public:
	template<typename T>
	void 	subscribe(const String& topic, const std::function<void (const T&)>& callback) {
		doSubscribe(topic, make_callback(callback));
	}

	template<typename T>
	void	publish(const String& topic, const T& val) {
		doPublish(topic, to_json<T>(val));
	}

	static MQTTClient* instance(Client& client);
	friend class MQTTChildClient;
};

template<typename T>
payload_callback_t	make_callback(const std::function<void (const T&)>& callback) {
	return [callback] (const String& payload) {
		callback(from_string<T>(payload));
	};
}

template<typename T, typename C>
std::function<void (const T&)> make_subscriber(const C& cb) {
	return std::function<void (const T&)>(cb);
}

typedef std::unique_ptr<MQTTClient>		MQTTClientPtr;
#endif