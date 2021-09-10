#pragma once

#include <ACS712.h>
#include <AdcNode.hpp>

// Create Node for read current form adc (ACS7128)
class ACS7128Node : public AdcNode {
private:
    ACS712 mAcs712;

public:

    using AdcNode::AdcNode;

    ACS7128Node(const char *id,
                const char *name,
                uint32_t readInterval = 1 * 1000UL,
                float sendOnChangeAbs = 0.06f,
                ACS712_type type = ACS712_05B)
            : AdcNode(id, name, readInterval, sendOnChangeAbs, nullptr, nullptr, [](bool value) { return true; }),
              mAcs712(type, A0) {}

    uint16_t calibrate() {

        auto zero = mAcs712.calibrate();

        Homie.getLogger() << "Current sensor middle/zero point: " << zero << endl;
        Homie.getLogger() << "IMPORTANT: run without any load attached for a correct calibration!" << endl;

        return zero;
    }

    void setup() override {

        calibrate();

        advertise(getId())
                .setDatatype("float")
                .setFormat("0:5.00")
                .setUnit(cUnitAmpere);

        advertise("power")
                .setDatatype("float")
                .setFormat("0:1100.00")
                .setUnit(cUnitWatt);
    }

    float readMeasurement() override {
        return mAcs712.getCurrentAC();
    }

    void sendMeasurement(float current) const override {
        AdcNode::sendMeasurement(current);

        if (Homie.isConnected()) {
            setProperty("power").send(String(220.0f * current));
        }
    }
};