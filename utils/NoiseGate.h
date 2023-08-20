#pragma once
#ifndef DSY_NOISE_GATE_H
#define DSY_NOISE_GATE_H

#include <stdlib.h>
#include "EnvelopeDetector.h"
#include "AREnvelope.h"

// TODO: until I'm able to get cmake to correctly import DaisySP,
// I'm doing this hack to mock out dsp methods for the test environment.
#ifdef DSYSP_H
#include "Utility/dsp.h"

#define POW_10F(v) daisysp::pow10f(v)

#else
#include <cmath>

#define POW_10F(v) powf(10.0f, v)
#endif

class NoiseGate
{
public:
    NoiseGate() {}
    ~NoiseGate() {}

    /**
     * Initializes a new NoiseGate instance.
     */
    void Init(int sampleRate, float onThresholdDB, float offThresholdDB,
              float holdTimeSeconds = 0,
              float attackTimeSeconds = 0.025 /* 25ms */,
              float releaseTimeSeconds = 0.02 /* 20ms */,
              float envelopeWindowSeconds = 0.05 /* 50ms */)
    {
        sampleRate_ = sampleRate;
        envelopeDetector_.Init(envelopeWindowSeconds * sampleRate_);
        envelope_.Init(sampleRate, attackTimeSeconds, releaseTimeSeconds);
        SetThresholds(onThresholdDB, offThresholdDB);
        SetHoldTime(holdTimeSeconds);

        // initialize state
        thresholdCount_ = 0;
        gateOpen_ = false;
    }

    /**
     * Simplified initializer with some sensible defaults.
     */
    void Init(int sampleRate, float thresholdDb)
    {
        Init(sampleRate, thresholdDb + 1, thresholdDb);
    }

    /**
     * Update buffer `in` in place, detecting the volume and scaling the values by 0 to 1.
     */
    void ProcessBlock(float *in, float *out, size_t size)
    {
        float env;
        for (size_t i = 0; i < size; i += 2)
        {
            // NOTE: even though we know the input is stereo, we're only running detection
            // on the first channel
            env = Process(in[i]);
            out[i] = in[i] * env;
            out[i + 1] = in[i + 1] * env;
        }
    }

    /**
     * Return the gain factor for one step.
     */
    inline float Process(float in)
    {
        float h = envelopeDetector_.Process(fabsf(in)); // detect envelope

        if ((gateOpen_ && h < offThreshold_) || (!gateOpen_ && h > onThreshold_))
        {
            thresholdCount_++;
        }
        if (thresholdCount_ >= holdSamples_)
        {
            gateOpen_ = !gateOpen_; // toggle
            thresholdCount_ = 0;    // reset counter
        }

        return envelope_.Process(gateOpen_);
    }

    // Getters and Setters

    bool IsOpen() { return gateOpen_; }

    void SetThreshold(float thresholdDB)
    {
        SetThresholds(thresholdDB + 1, thresholdDB);
    }

    void SetThresholds(float onThresholdDB, float offThresholdDB)
    {
        // store thresholds as scale factor
        onThreshold_ = POW_10F(onThresholdDB / 10.0f);
        offThreshold_ = POW_10F(offThresholdDB / 10.0f);
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
