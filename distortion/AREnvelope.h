#pragma once
#ifndef DSY_AR_ENVELOPE_H
#define DSY_AR_ENVELOPE_H

#include <stdlib.h>

enum AREnvelopeState
{
    ARENV_ATTACK,
    ARENV_ON,
    ARENV_RELEASE,
    ARENV_OFF
};

// A simple Attack-Release envelope.
// Ramp a binary signal smoothly with attack and release times.
// Must call Process for each sample with the current gate state.
class AREnvelope
{
public:
    AREnvelope() {}
    ~AREnvelope() {}

    void Init(int sampleRate, float attackTimeSeconds, float releaseTimeSeconds)
    {
        sampleRate_ = sampleRate;
        state_ = ARENV_OFF;
        sampleCounter_ = 0;
        SetAttackTime(attackTimeSeconds);
        SetReleaseTime(releaseTimeSeconds);
    }

    void SetAttackTime(float attackTimeSeconds)
    {
        attackSamples_ = attackTimeSeconds * sampleRate_;
        if (sampleCounter_ > attackSamples_)
            sampleCounter_ = attackSamples_; // deal with jumps
    }

    void SetReleaseTime(float releaseTimeSeconds)
    {
        releaseSamples_ = releaseTimeSeconds * sampleRate_;
        if ((sampleCounter_ - releaseSamples_) < 0)
            sampleCounter_ = 0; // deal with jumps
    }

    AREnvelopeState GetState() { return state_; }

    // take in a binary signal and return a value from 0-1
    float Process(bool on)
    {
        switch (state_)
        {
        case ARENV_ATTACK:
            if (!on)
            {
                // turn off (don't reset counter)
                state_ = ARENV_RELEASE;
            }
            else if (sampleCounter_ >= attackSamples_)
            {
                state_ = ARENV_ON; // full on
            }
            else
            {
                sampleCounter_++; // step
            }
            return sampleCounter_ / attackSamples_;
        case ARENV_ON:
            if (!on)
            {
                // turn off
                state_ = ARENV_RELEASE;
                sampleCounter_ = releaseSamples_;
            }
            return 1.0f; // on
        case ARENV_RELEASE:
            if (on)
            {
                // turn on (don't reset counter)
                state_ = ARENV_ATTACK;
            }
            else if ((sampleCounter_ - releaseSamples_) <= 0)
            {
                state_ = ARENV_ON; // full on
            }
            else
            {
                sampleCounter_--; // step
            }
            return sampleCounter_ / attackSamples_;
        case ARENV_OFF:
            if (on)
            {
                // turn on
                state_ = ARENV_ATTACK;
                sampleCounter_ = 0;
            }
            return 0.0f; // off
        default:
            return 0.0f; // undefined behavior
        }
    }

private:
    int sampleRate_;
    int attackSamples_;
    int releaseSamples_;
    AREnvelopeState state_;
    int sampleCounter_;
};

#endif // DSY_AR_ENVELOPE_H