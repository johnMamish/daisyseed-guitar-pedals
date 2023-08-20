#pragma once
#ifndef DSY_ENVELOPE_DETECTOR_H
#define DSY_ENVELOPE_DETECTOR_H

#include <stdlib.h>
#include "Utility/dsp.h"

#ifndef min
#define min(a, b) ((a < b) ? a : b)
#endif

// Simple implementation based on Max MSP `rampsmooth~`
class EnvelopeDetector
{
public:
  EnvelopeDetector() {}
  ~EnvelopeDetector() {}

  void Init(size_t rampSamples)
  {
    state_ = 0;
    step_ = 1.0f / (float)(rampSamples < 1 ? 1 : rampSamples);
  }

  void ProcessBlock(float *in, float *out, size_t size)
  {
    for (size_t i = 0; i < size; i++)
    {
      *out = Process(*in);
    }
  }

  inline float Process(float in)
  {
    float diff = in - state_;
    if (diff >= 0)
      state_ += min(diff, step_);
    else
      state_ -= min(fabsf(diff), step_);
    return state_;
  }

private:
  float state_;
  float step_;
};

#endif // DSY_ENVELOPE_DETECTOR_H