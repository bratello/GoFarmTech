#include  <Arduino.h>
/*
//Specify settings pins here
#define SETTINGS_KNOB D3
#define SETTINGS_LED 2
*/
#include  <LogicalValue.h>
#include  <RelayValue.h>
#include  <DHTValue.h>
#include  <SoilMoistureValue.h>
#include  <MQTTNetworkPlugin.h>

////////////////////////////////////////////////
// Main Device Class
class ESP8266Timer : public MQTTNetworkPlugin {
	////////////////////////////////////////////////
	// Specify device name for logs & device description
	LOGGABLE_MODULE_NAME(ESP8266Timer)

////////////////////////////////////////////////
// Sensors & external devices definition
// Every sensor or relay introduced as value
public:
	////////////////////////////////////////////////
	// Relay values 
	// Value/attributes can be changed from mqtt broker
    DEFINE_RELAY(WateringRelay, D1) wateringRelay;
    DEFINE_RELAY(ConditionRelay, D2) conditionRelay;
    
	////////////////////////////////////////////////
	// Sensors values
	// Aattributes can be changed from mqtt broker
	DEFINE_DHT11(RoomTemp, D7, true, 18, 30) roomTemp;
    DEFINE_DHT11(RoomHumidity, D7, false, 30, 65) roomHumidity;
    DEFINE_SOILSENSOR(SoilMoistureSensor, A0, 10, 40) soilMoisture;
	
	////////////////////////////////////////////////
	// Logical values. Useful for device logic
	// Value/attributes can be changed from mqtt broker
	DEFINE_LOGICAL_VALUE(WateringCapacity, long, 0, 200, 400) wateringCapacity;
	DEFINE_LOGICAL_FLAG(SomeProcessFlag, false) someProcessFlag;

	// Default constructor
    ESP8266Timer() : MQTTNetworkPlugin() {
    	//Settings::instance()->deviceVersion = 1;
    }

////////////////////////////////////////////////
// Helper instance members
protected:
	int conditionalStepCount = 0;

////////////////////////////////////////////////
// Device & sensors setup section
protected:

	////////////////////////////////////////////////
	// Propagate sensors and relays to the framwork
	virtual void  setupValues() {
		setupValue(wateringRelay);
		setupValue(conditionRelay);
		setupValue(roomTemp);
		setupValue(roomHumidity);
		setupValue(soilMoisture);
		setupValue(wateringCapacity);
		setupValue(someProcessFlag);
	}

	////////////////////////////////////////////////
	// Install timer tasks
	virtual void  setupTimer(TimerValue& timer) {
		/////////////////////////////////////////////////////////////
		// Create wateringTask by specifiyng the string cron settings
		// See TimerValue.on method interface
		timer.on("wateringTask", "25.*.*.*.*:20", [this] () {
			// onStartTask callback
			wateringRelay.setValue(true);
		}, [this] () {
			// onEndTask callback
			wateringRelay.setValue(false);
		});

		///////////////////////////////////////////////////////////////////////////
		// Create conditionTask by specifiyng the TimerSlot array as cron settings
		// See TimerSlot class interface
		timer.on("conditionTask", { 
				{0, TIME_INTERVAL::DAY, 10, 25, TimerSlot::ALL_DAYS, TimerSlot::ALL_MONTHS} 
			}, [this] () {
				// onStartTask callback
				conditionRelay.setValue(true);
			}, [this] () {
				// onEndTask callback
				conditionRelay.setValue(false);
			}
		);

		/////////////////////////////////////////////////////////////////////////////////////////////////
		// Create wateringPipeline multiple steps task by specifiyng the TimerSlot array as cron settings
		// TimerValue::onPipeline() method returns the TimerTaskPipeline class
		// Every additional step can be specified by calling the operator ()
		// See TimerTaskPipeline::operator ()
		timer.onPipeline("wateringPipeline",{ 
			{0, TIME_INTERVAL::DAY, 30, TIME_INTERVAL::MINUTE, TimerSlot::ALL_DAYS, TimerSlot::ALL_MONTHS} 
		})("tenSecStep", 10, [this] (const String& name) {
			// onStartStep callback
			// tenSecStep step running 10 sec
			// Do your step logic here
			LOGGER(info(name)(" step execution"))
			return [this, name] (const step_command_t& /*cmd*/) {
				// onStepState callback for complete or query the step
				// Dispose consumed data - onStepEnd notification received
				LOGGER(info(name)(" step finished"))
				return step_status_t::FINISHED;
			};
		})("folowTaskStep", FOLLOW_BY_TASK_DURATION, [this](const String& name) {
			// onStartStep callback
			// folowTaskStep step running according to the wateringPipeline task duration settings
			// Do your step logic here
			LOGGER(info(name)(" followed by task step execution"))
			return [this, name] (const step_command_t& /*cmd*/) {
				// onStepState callback for complete or query the step
				// Dispose consumed data - onStepEnd notification received
				LOGGER(info(name)(" followed by task step finished"))
				return step_status_t::FINISHED;
			};
		})("conditionalStep", [this](const String& name) {
			// onStartStep callback
			// conditionalStep will be completed by his onStepState, see below
			// Do your step logic here
			// Be carefull with the local scope variable passed to the onStepState callback, compiler can optimize unusable local variable
			conditionalStepCount = 0;
			LOGGER(info(name)(" conditional step execution: ")(conditionalStepCount))
			return [this, name] (const step_command_t& cmd) {
				// onStepState callback for complete or query the step
				// Check the cmd first
				if(step_command_t::CAN_FINISH_STEP == cmd) {
					// Handle callback only in CAN_FINISH_STEP case
					LOGGER(info(name)(" CAN_FINISH_STEP query received: ")(conditionalStepCount))
					// Check soilMoisture/Whatever value here, e.g. soilMoisture.isMinValue() or other thigs
					if(conditionalStepCount++ < 10) {
						// Continue the task execution
						LOGGER(info(name)(" the soil still dry: ")(conditionalStepCount))
						return step_status_t::STARTED;
					}
				}
				// Dispose consumed data - onStepEnd notification received
				LOGGER(info(name)(" conditional step finished"))
				return step_status_t::FINISHED;
			};
		}).onBeforeStep([this] (const String& name) {
			// onBeforeStep callback called before every step execution
			// Good idea to prepare data before any step execution globally
			// You can switch off relays, vents or leds before the next step
			LOGGER(info("Before ")(name)(" step"))
		}).onAfterStep([this] (const String& name) {
			// onAfterStep callback called after every step execution
			// Good idea for data cleanup after any step execution globally
			LOGGER(info("After ")(name)(" step"))
		});
	}

	////////////////////////////////////////////////
	// Setup sensors logic
	virtual void  setupDeviceLogic() {
		// Switch ON the conditionRelay when the roomHumidity max threshold reached 
		roomHumidity.onMaxValReached([this] () {
			info("Room Humidity reached");
			conditionRelay.setValue(true);
		});

		// Switch OFF the conditionRelay when the roomHumidity normal threshold reached 
		roomHumidity.onOKValReached([this] () {
			info("Room Humidity OK");
			conditionRelay.setValue(false);
		});

		// Switch OFF any other related devices when the wateringRelay state was changed
		// Same things you can do with other values like sensors or switches
		wateringRelay.onChanged([this] (bool newVal, bool oldVal) {
			if(newVal) {
				info("Start watering");
			} else {
				info("Stop watering");
			}
		});

		// Switch OFF any other related devices when the conditionRelay state was changed
		conditionRelay.onChanged([this] (bool newVal, bool oldVal) {
			if(newVal) {
				info("Start Condition");
			} else {
				info("Stop Condition");
			}
		});
	}

	////////////////////////////////////////////////
	// Setup Serial for the logs
	virtual void  setupLogs() {
		Serial.begin(115200);
		//enableModuleLog(4);
		MQTTNetworkPlugin::setupLogs();
	}

	////////////////////////////////////////////////
	// Setup helper infrastructures if needed
	virtual void  setupInfra() {
		//info("DESCRIPTION: ")(getDescription().toJSON());
		//info("SETTINGS: ")(Settings::instance()->toJSON());
	}
};

RunnablePtr device;

void setup() {
	//////////////////////////////////////////////////////////
	// Create device instance
	device = RunnablePtr(new ESP8266Timer());
	// Setup device with all related sensors and infrastrucures
	device->setup();
}

void loop() {
	////////////////////////////////////////////////
	// Loop the device sensors and infrastructures
	device->loop();
}