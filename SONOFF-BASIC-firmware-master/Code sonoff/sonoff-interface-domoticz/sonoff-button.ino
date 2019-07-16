/*
  SONOFF BASIC: firmware
  More info: https://github.com/tschaban/SONOFF-BASIC-firmware
  LICENCE: http://opensource.org/licenses/MIT
  2016-10-27 tschaban https://github.com/tschaban
*/

#include "sonoff-button.h"


SonoffButton::SonoffButton() {
  pinMode(BUTTON, INPUT_PULLUP);
  start();
}

void SonoffButton::start() {
  buttonTimer.attach(0.05, callbackButton);
  if (Configuration.debugger) Serial << endl << "INFO: Button has been turned on";
}

void SonoffButton::stop() {
  buttonTimer.detach();
  if (Configuration.debugger) Serial <<  endl << "WARN: Button has been turned off";
}


boolean SonoffButton::isPressed() {
  return !digitalRead(BUTTON);
}

void SonoffButton::pressed() {
  counter++;
}

void SonoffButton::reset() {
  counter = 0;
}

boolean SonoffButton::accessPointTrigger() {
  return counter == 80 ? true : false;
}

boolean SonoffButton::configurationTrigger() {
  return counter > 20 && counter < 80 ? true : false;

}

boolean SonoffButton::relayTrigger() {
  return counter > 1 && counter <= 10 ? true : false;
}

void callbackButton() {

  if (Button.isPressed() && !Button.accessPointTrigger()) {
    Button.pressed();
  } else if (Button.isPressed() && Button.accessPointTrigger()) {
    Button.stop();
    if (Configuration.mode == MODE_ACCESSPOINT) {
      Sonoff.run();
    } else {
      Eeprom.saveMode(MODE_ACCESSPOINT);
      delay(10);
      ESP.restart();
    }
  } else {
    if (Configuration.mode == MODE_SWITCH && Button.relayTrigger()) { // short press. Relay state change
      Relay.toggle();
      if (Configuration.interface == INTERFACE_MQTT) {
         MqttInterface.publishRelayState();
      } else if (Configuration.interface == INTERFACE_HTTP && Configuration.domoticz_publish_relay_state) { // Publish change to Domoticz if configured
        DomoticzInterface.publishRelayState(Relay.get());
      }
    } else if (Button.configurationTrigger()) { // 4-6 sec 
      Sonoff.toggle();
    }
    Button.reset();
  }
}
