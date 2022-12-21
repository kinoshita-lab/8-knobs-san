#include <Arduino.h>

#include "AnalogMultiplexer74Hc4051.h"

enum
{
    kPinNEnable = 3,
    kPinS0      = 5,
    kPinS1      = 6,
    kPinS2      = 7,

    kAnalogIn = 0,
};

kinoshita_lab::AnalogMultiplexer74HC4051 multiplexer1(kPinNEnable, kPinS0, kPinS1, kPinS2, kAnalogIn);

float float_buf1[kinoshita_lab::AnalogMultiplexer74HC4051::kNumInputs] = {0.f};

int buf1[kinoshita_lab::AnalogMultiplexer74HC4051::kNumInputs] = {0};

void setup()
{
    Serial.begin(115200);

    // put your setup code here, to run once:

    multiplexer1.begin();

    for (auto i = 0; i < kinoshita_lab::AnalogMultiplexer74HC4051::kNumInputs; i++) {
        const auto value = multiplexer1.getValue(i) >> 1;
        buf1[i]          = value;
    }
}

void loop()
{
    // put your main code here, to run repeatedly:
    multiplexer1.update();

    for (int i = 0; i < kinoshita_lab::AnalogMultiplexer74HC4051::kNumInputs; i++) {
        const auto val   = multiplexer1.getFilteredValue(i) >> 1;
        const auto delta = abs(buf1[i] - val);
        if (delta) {
            buf1[i] = val;

            Serial.print("scan value[");
            Serial.print(i);
            Serial.print("] = ");
            Serial.println(val);
        }
    }
}
