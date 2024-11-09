#include <Homie.h>
#include <Wire.h>
#include <BMx280I2C.h>
#include <jled.h>

#define DETECTA_PIN 0
#define RELEY_PIN 12
#define LED_PIN 13

const int PIN_BELL = A0;
unsigned long lastNotify = 0;
unsigned long intervalNotification = 8000; // millisecs

bool sendMessage = false;

auto led = JLed(LED_PIN).Breathe(2000).DelayAfter(1000).Forever();

HomieNode bellNode("bell", "Bell", "bell");

void onHomieEvent(const HomieEvent &event)
{
  switch (event.type)
  {
  case HomieEventType::STANDALONE_MODE:
    Serial << "Standalone mode started" << endl;
    led.Blink(100, 30000).Forever();
    break;
  case HomieEventType::CONFIGURATION_MODE:
    Serial << "Configuration mode started" << endl;
    led.Blink(500, 500).Forever();
    ;
    break;
  case HomieEventType::NORMAL_MODE:
    Serial << "Normal mode started" << endl;
    led.Blink(100, 30000).Forever();
    break;
  case HomieEventType::OTA_STARTED:
    Serial << "OTA started" << endl;
    break;
  case HomieEventType::OTA_PROGRESS:
    Serial << "OTA progress, " << event.sizeDone << "/" << event.sizeTotal << endl;
    break;
  case HomieEventType::OTA_FAILED:
    Serial << "OTA failed" << endl;
    break;
  case HomieEventType::OTA_SUCCESSFUL:
    Serial << "OTA successful" << endl;
    break;
  case HomieEventType::ABOUT_TO_RESET:
    Serial << "About to reset" << endl;
    break;
  case HomieEventType::WIFI_CONNECTED:
    Serial << "Wi-Fi connected, IP: " << event.ip << ", gateway: " << event.gateway << ", mask: " << event.mask << endl;
    led.Breathe(2000).Forever();
    break;
  case HomieEventType::WIFI_DISCONNECTED:
    Serial << "Wi-Fi disconnected, reason: " << (int8_t)event.wifiReason << endl;
    break;
  case HomieEventType::MQTT_READY:
    Serial << "MQTT connected" << endl;
    led.Blink(100, 30000).Forever();
    break;
  case HomieEventType::MQTT_DISCONNECTED:
    Serial << "MQTT disconnected, reason: " << (int8_t)event.mqttReason << endl;
    led.Breathe(1000).Forever();
    break;
    /*
    case HomieEventType::MQTT_PACKET_ACKNOWLEDGED:
      Serial << "MQTT packet acknowledged, packetId: " << event.packetId << endl;
      break;
    case HomieEventType::READY_TO_SLEEP:
      Serial << "Ready to sleep" << endl;
      break;
    case HomieEventType::SENDING_STATISTICS:
      Serial << "Sending statistics" << endl;
      break;
    */
  }
}

bool switchOnHandler(const HomieRange &range, const String &value)
{
  if (value != "true" && value != "false")
    return false;

  bool on = (value == "false");
  digitalWrite(RELEY_PIN, on ? HIGH : LOW);
  bellNode.setProperty("relay").send(value);
  Homie.getLogger() << "Switch is " << (on ? "off" : "on") << endl;

  return true;
}

void loopHandler()
{
  int bellValue = digitalRead(DETECTA_PIN);

  // Campainha
  if (bellValue != 1)
  {
    sendMessage = true;
    if ((lastNotify + intervalNotification) < millis())
    {
      lastNotify = millis();
      Homie.getLogger() << "Ring is now ringing" << endl;
      bellNode.setProperty("ring").send("true");
    }
  }
  else if (sendMessage)
  {
    if ((lastNotify + intervalNotification) < millis())
    {
      lastNotify = millis();
      Homie.getLogger() << "Ring is now in silence" << endl;
      bellNode.setProperty("ring").send("false");
      sendMessage = false;
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial << endl
         << endl;

  pinMode(DETECTA_PIN, INPUT_PULLUP);

  pinMode(RELEY_PIN, OUTPUT);
  digitalWrite(RELEY_PIN, HIGH);

  Homie_setFirmware("bell-detector", "2.0.1");
  Homie.onEvent(onHomieEvent);
  Homie
      .setLoopFunction(loopHandler);

  bellNode.advertise("ring").setName("isRinging").setDatatype("boolean");
  bellNode.advertise("relay").setName("Relay").setDatatype("boolean").settable(switchOnHandler);
  Homie.setup();
}

void loop()
{
  led.Update();
  Homie.loop();
}