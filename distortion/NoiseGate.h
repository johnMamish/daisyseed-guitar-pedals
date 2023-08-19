#pragma once
#ifndef DSY_NOISE_GATE_H
#define DSY_NOISE_GATE_H

#include <stdlib.h>
#include "EnvelopeDetector.h"
#include "AREnvelope.h"

class NoiseGate
{
public:
    NoiseGate() {}
    ~NoiseGate() {}

    /**
     * Initializes a new NoiseGate instance.
     */
    void Init(int sampleRate, float thresholdDB, float envelopeWindowSeconds = 0.05 /* 50ms */)
    {
        sampleRate_ = sampleRate;
        envelopeDetector_.Init(envelopeWindowSeconds * sampleRate_);
        // Set some sensible defaults
        envelope_.Init(sampleRate, 0.085, 0.085);
        SetThreshold(thresholdDB);
        SetHoldTime(0.085); // 85ms

        // initialize state
        thresholdCount_ = 0;
        gateOpen_ = false;
    }

    /**
     * Update buffer `in` in place, detecting the volume and scaling the values by 0 to 1.
     */
    void ProcessBlock(float *in, size_t size)
    {
        float h, env;

        for (size_t i = 0; i < size; i += 2)
        {
            // NOTE: even though we know the input is stereo, we're only detecting on the first
            // channel
            h = envelopeDetector_.Process(fabsf(in[i])); // detect envelope

            if ((gateOpen_ && h < offThreshold_) || (!gateOpen_ && h > onThreshold_))
            {
                thresholdCount_++;
            }
            if (thresholdCount_ >= holdSamples_)
            {
                gateOpen_ = !gateOpen_; // toggle
                thresholdCount_ = 0;    // reset counter
            }

            env = envelope_.Process(gateOpen_);
            in[i] = in[i] * env;
            in[i + 1] = in[i + 1] * env;
        }
    }

    // Getters and Setters

    void SetThreshold(float thresholdDB)
    {
        SetThresholds(thresholdDB, thresholdDB - 10);
    }

    void SetThresholds(float onThresholdDB, float offThresholdDB)
    {
        // store thresholds as scale factor
        onThreshold_ = daisysp::pow10f(onThresholdDB / 10);
        offThreshold_ = daisysp::pow10f(offThresholdDB / 10);
    }

    void SetHoldTime(float holdTimeSeconds)
    {
        holdSamples_ = holdTimeSeconds * sampleRate_;
    }

    void SetAttackTime(float attackTimeSeconds)
    {
        envelope_.SetAttackTime(attackTimeSeconds);
    }

    void SetReleaseTime(float releaseTimeSeconds)
    {
        envelope_.SetReleaseTime(releaseTimeSeconds);
    }

private:
    int sampleRate_;
    EnvelopeDetector envelopeDetector_;
    AREnvelope envelope_;

    float onThreshold_;  // value to exceed for holdSamples_ before turning gate on
    float offThreshold_; // value to fall below for holdSamples_ before turning gate off
    unsigned int holdSamples_;
    bool gateOpen_;
    size_t thresholdCount_;
};

#endif // DSY_NOISE_GATE_H
