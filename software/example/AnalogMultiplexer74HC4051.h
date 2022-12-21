/**
 * @file AnalogMultiplexer74Hc4051.h
 * @brief Simple library for 74HC4051 analog multiplexer.
 *
 * @author Kazuki Saita <saita@kinoshita-lab.com>
 *
 * Copyright (c) 2022 Kinoshita Lab. All rights reserved.
 *
 */

#pragma once
#ifndef ANALOGMULTIPLEXER74HC4051_H
#define ANALOGMULTIPLEXER74HC4051_H

#include <stdint.h>
#include <Arduino.h>

namespace kinoshita_lab
{
class AnalogMultiplexer74HC4051
{
public:
    enum
    {
        kNumInputs             = 8,
        kDefaultScanPeriodMsec = 1,
        kNumAvarage            = 1,
    };

    AnalogMultiplexer74HC4051(const int pin_nenable,
                              const int pin_s0,
                              const int pin_s1,
                              const int pin_s2,
                              const int analog_in,
                              const bool is_enable_active_low = true,
                              const int scan_period           = kDefaultScanPeriodMsec)
        : pins_(pin_nenable, pin_s0, pin_s1, pin_s2, analog_in),
          is_enable_active_low_(is_enable_active_low),
          scan_period_(scan_period)
    {
        pinMode(pins_.nenable, OUTPUT);
        pinMode(pins_.s0, OUTPUT);
        pinMode(pins_.s1, OUTPUT);
        pinMode(pins_.s2, OUTPUT);
    }

    float getAvaragedAnalogRead()
    {
        float result = 0.f;
        for (auto i = 0; i < kNumAvarage; i++) {
            result += analogRead(pins_.analog_in);
        }
        return result / kNumAvarage;
    }
    void begin()
    {
        selectInput(scan_index_);
        digitalWrite(pins_.nenable, is_enable_active_low_ ? LOW : HIGH);
        delay(1);

        for (auto i = 0; i < kNumInputs; i++) {
            selectInput(i);
            delay(1);
            scan_values_[i]     = getAvaragedAnalogRead();
            filters_[i].output_ = scan_values_[i];
        }
        last_scanned_millis_ = millis();
    }

    virtual ~AnalogMultiplexer74HC4051() = default;

    void update()
    {

        const unsigned long current_millis = millis();
        const unsigned long delta_millis   = abs(current_millis - last_scanned_millis_);

        if (delta_millis < scan_period_) {
            return;
        }
        last_scanned_millis_ = current_millis;
        const auto read      = getAvaragedAnalogRead();

        scan_values_[scan_index_] = read;
        filters_[scan_index_].update(read);

        // prepare for next
        scan_index_ = (scan_index_ + 1) % kNumInputs;
        selectInput(scan_index_);
        digitalWrite(pins_.nenable, is_enable_active_low_ ? LOW : HIGH);
    }

    void selectInput(const uint8_t index)
    {
        if (index >= kNumInputs) {
            return;
        }

        digitalWrite(pins_.s0, (index >> 0) & 0x01);
        digitalWrite(pins_.s1, (index >> 1) & 0x01);
        digitalWrite(pins_.s2, (index >> 2) & 0x01);
    }

    int getValue(const int index) const
    {
        return (int)scan_values_[index];
    }
    int getFilteredValue(const int index) const
    {
        return round(filters_[index].output_);
    }

protected:
    struct Pins
    {
        enum
        {
            kInvalidPinConfiguration = 0xff,
        };

        int nenable   = kInvalidPinConfiguration;
        int s0        = kInvalidPinConfiguration;
        int s1        = kInvalidPinConfiguration;
        int s2        = kInvalidPinConfiguration;
        int analog_in = kInvalidPinConfiguration;

        Pins() = default;
        Pins(const int pin_nenable,
             const int pin_s0,
             const int pin_s1,
             const int pin_s2,
             const int pin_analog_in)
            : nenable(pin_nenable), s0(pin_s0), s1(pin_s1), s2(pin_s2), analog_in(pin_analog_in)
        {
        }
    };
    class OnePoleIIR
    {
    public:
        static constexpr float kDefaultAlpha = 0.8f; ///< Tuned for  1ms, 1 average, 512 resolution

        OnePoleIIR() : alpha_(kDefaultAlpha), output_(0.0f) {}

        virtual ~OnePoleIIR() = default;

        void setLastValue(const float value)
        {
            output_ = value;
        }

        void update(const float input)
        {
            const auto output = alpha_ * input + (1.0f - alpha_) * output_;
            output_           = output;
        }
        float alpha_  = kDefaultAlpha;
        float output_ = 0.0f;
    };
    // configurations
    OnePoleIIR filters_[kNumInputs];

    Pins pins_;
    bool is_enable_active_low_ = true;
    unsigned long scan_period_ = kDefaultScanPeriodMsec;

    // state variables
    unsigned long last_scanned_millis_ = 0;
    int scan_index_                    = 0;
    float scan_values_[kNumInputs]     = {0.f};

private:
    AnalogMultiplexer74HC4051() = default;
};
}

#endif // ANALOGMULTIPLEXER74HC4051_H
