// SWECS SwitchExternalControl&State

#define AP_NAME "BDIot"
#define FW_NAME "SWECS"
#define FW_VERSION "0.0.6b"

#include <Homie.h>

// Nodes libs
#include <SwitchNode.hpp>
#include <BinarySensorNode.hpp>
#include <ButtonNode.hpp>

/**
 * TODO:
 * * create a PR to fix the value of Homie.isConnected() on first first run of loop...see BootNormal.cpp.patch
 */

#ifndef SERIAL_SPEED
#define SERIAL_SPEED 115200
#endif

// Pins used
#define PIN_OUT_COMMAND 12
#define PIN_IN_EXT_CONTROL 13
#define PIN_IN_EXT_STATE 14
#define SETTING_EXT_CONTROL_MODE 0

// Device custom settings
HomieSetting<long> settingExtControlMode("externalControlMode", "Button (external control) mode: 0:toggle, 1:impulsive, 2:follow");

enum class extControlMode : uint8_t {
    TOGGLE = 0,
    IMPULSIVE,
    FOLLOW
};

// Nodes instances
SwitchNode commandNode("switch", "Switch", PIN_OUT_COMMAND);
ButtonNode extControlNode("button", "Button", PIN_IN_EXT_CONTROL, INPUT_PULLUP, 10, LOW, 3, 1000);
#ifdef EXT_STATE
BinarySensorNode extStateNode("state", "State", PIN_IN_EXT_STATE, INPUT_PULLUP, 10);
#endif

/* HANDLERS / CALLBACKS */

bool broadcastHandler(const String &topic, const String &value) {
    Homie.getLogger() << "Received broadcast " << topic << ": " << value << endl;
    return true;
}

bool globalInputHandler(const HomieNode &node, const HomieRange &range, const String &property, const String &value) {
    Homie.getLogger() << "Received/published an input msg on node " << node.getId() << ": " << property << " = " << value << endl;
    return false;
}

bool extControlHandler(const ButtonEvent &event) {

    auto mode = static_cast<extControlMode>(settingExtControlMode.get());

    switch (event.type) {
        case ButtonEventType::PRESS:

            switch (mode) {
                case extControlMode::IMPULSIVE:
                    commandNode.stopTimeout();
                case extControlMode::TOGGLE:
                    commandNode.setState(!commandNode.getState());
                    break;
                case extControlMode::FOLLOW:
                    commandNode.setState(true);
                    break;
            }

            break;
        case ButtonEventType::RELEASE:

            switch (mode) {
                case extControlMode::TOGGLE:
                    commandNode.setState(!commandNode.getState());
                    break;
                case extControlMode::IMPULSIVE:
                    if (event.duration.previous >= 1000) {
                        commandNode.setTimeout(event.duration.previous * 10 / 1000, !commandNode.getState());
                    }
                    break;
                case extControlMode::FOLLOW:
                    commandNode.setState(false);
                    break;
            }

            break;
        case ButtonEventType::MULTI_PRESS_COUNT:
        case ButtonEventType::MULTI_PRESS_INTERVAL:

            switch (mode) {
                case extControlMode::TOGGLE:
                    if (event.pressCount > 1) {
                        commandNode.setTimeout(event.pressCount * 10, !commandNode.getState());
                    }
                    break;
                default:
                    break;
            }

            break;
        default:
            break;
    }

    return true;
}

bool commandHandler(bool state) {

    Homie.getLogger() << "Command: " << state << endl;
    // eventually stop active running timer
    commandNode.stopTimeout();

    return true;
}

#ifdef EXT_STATE
bool extStateHandler(bool state) {

    Homie.getLogger() << "External state: " << state << endl;
    return true;
}
#endif

void setup() {
    Serial.begin(SERIAL_SPEED);

    // HOMIE SETUP
    Homie_setBrand(AP_NAME);                    // Brand in fw, mqtt client name and AP name in configuration mode
    Homie_setFirmware(FW_NAME, FW_VERSION);     // Node name and fw version (for OTA stuff)

    // Device custom settings: default and validation
    settingExtControlMode.setDefaultValue(SETTING_EXT_CONTROL_MODE).setValidator([](uint8_t candidate) {
        return 0 <= candidate && candidate <= 2;
    });

    //Homie.setSetupFunction();
    Homie.setBroadcastHandler(broadcastHandler);
    Homie.setGlobalInputHandler(globalInputHandler);

    // NODES SETUP
    commandNode.setOnSetFunc(commandHandler);

    // call extControlNode.loop() even when not connected (but still in normal mode)
    // i.e. check External Control state also when offline (controlling CommandNode)
    extControlNode.setRunLoopDisconnected(true);
    extControlNode.setOnChangeFunc(extControlHandler);

    #ifdef EXT_STATE
    commandNode.setGetStateFunc([=]() { return extStateNode.readMeasurement(); });
    extStateNode.setOnChangeFunc(extStateHandler);
    #endif

    Homie.setup();
}

void loop() {
    Homie.loop();

    // Uncomment for ssl in case of instability (ie disconnection)
    // https://github.com/homieiot/homie-esp8266/issues/640
    // try delay(100); or yield();
}