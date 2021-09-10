#pragma once

#include <SwitchNode.hpp>
#include <BinarySensorNode.hpp>

/*
 * SwitchNode with external state and impulsive support
 * note: this is a Multiple Inheritance Diamond problem:
 * * allow calling BaseNode (ie HomieNode) constructor one time => create a single node
 * * HomieNode methods (eg. setup(), onReadyToOperate(), handleInput(), loop()) must be
 *   redefined when appear in both classes (ie SwitchNode, BinarySensor)
 */

class AdvSwitchNode : virtual public SwitchNode, virtual protected BinarySensorNode {
protected:
    Ticker mTickerImp;
    uint16_t mImpulseLengthMs;
public:
    AdvSwitchNode(const char *id,
                  const char *name,
                  int8_t pin,
                  int8_t statePin,
                  uint8_t pinValueForTrue = HIGH,
                  uint8_t statePinValueForTrue = LOW,
                  uint16_t impulseLengthMs = 200,
                  const OnSetFunc &onSetFunc = [](bool value) { return true; },
                  const GetStateFunc &getStateFunc = nullptr,
                  const SetHwStateFunc &setHwStateFunc = nullptr,
                  const SendStateFunc &sendStateFunc = nullptr)
            : BaseNode(id, name, "switch"),
              SwitchNode("command", name, pin, pinValueForTrue, onSetFunc, getStateFunc, setHwStateFunc, sendStateFunc),
              BinarySensorNode("state", name, statePin, INPUT_PULLUP, 10, statePinValueForTrue),
              mImpulseLengthMs(impulseLengthMs) {}

    void setState(bool value) override {

        if (!mImpulseLengthMs) {
            // normal switch
            SwitchNode::setState(value);

        } else if (getState() != value && onSet(value)) {
            // Impulsive switch

            Homie.getLogger() << BaseNode::cIndent << "generate impulse" << endl;

            impulse();
        }
    }

    void impulse(bool rise = true) {

        setHwState(rise);

        if (Homie.isConnected()) sendState(rise);

        if (rise) {
            mTickerImp.once_ms_scheduled(mImpulseLengthMs, [=]() {
                impulse(false);
            });
        }
    }

    bool getState() override {
        return BinarySensorNode::readMeasurement();
    }

    SensorInterface<bool> &setOnStateChangeFunc(const OnChangeFunc &func) {
        return SensorInterface::setOnChangeFunc(func);
    }
};
