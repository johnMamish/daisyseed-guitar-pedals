#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"
#include "NoiseGate.h"

using namespace daisy;
using namespace daisysp;
using namespace terrarium;

// Declare a local daisy_petal for hardware access
DaisyPetal hw;
bool bypass;

Parameter blendCtrl, noiseGateCtrl;
Led led1, led2;

CrossFade blend;
NoiseGate noiseGate;

inline void silence(const float *in, float *out, unsigned int size)
{
    for (size_t i = 0; i < size; i++)
    {
        out[i] = 0;
    }
}

inline void passthru(const float *in, float *out, unsigned int size)
{
    for (size_t i = 0; i < size; i += 2)
    {
        out[i] = in[i];         // left
        out[i + 1] = in[i + 1]; // right
    }
}

inline void distort(const float *in, float *out, unsigned int size)
{
    for (size_t i = 0; i < size; i += 2)
    {
        // distort
        out[i] = sqrt(in[i]);
        out[i + 1] = sqrt(in[i + 1]);

        // apply wet-dry blend
        out[i] = blend.Process((float &)in[i], out[i]);
        out[i + 1] = blend.Process((float &)in[i + 1], out[i + 1]);
    }
}

void callback(const float *in, float *out, unsigned int size)
{
    hw.ProcessAllControls();
    led1.Update();
    led2.Update();
    blend.SetPos(blendCtrl.Process());
    noiseGateCtrl.Process();
    noiseGate.SetThresholds(noiseGateCtrl.Value() + 2, noiseGateCtrl.Value());
    hw.switches[Terrarium::FOOTSWITCH_2].Debounce();

    if (hw.switches[Terrarium::FOOTSWITCH_1].RisingEdge())
    {
        bypass = !bypass;
        led1.Set(bypass ? 0.0f : 1.0f);
    }

    if (bypass)
    {
        passthru(in, out, size);
    }
    else if (hw.switches[Terrarium::FOOTSWITCH_2].Pressed())
    {
        silence(in, out, size); // momentary killswitch
    }
    else
    {
        distort(in, out, size);
    }

    // apply noise gate (in place)
    noiseGate.ProcessBlock(out, out, size);
    led2.Set(noiseGate.IsOpen() ? 1.0f : 0.0f);
}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(12);
    blend.Init(CROSSFADE_CPOW);
    noiseGate.Init(hw.AudioSampleRate(), 0);

    blendCtrl.Init(hw.knob[Terrarium::KNOB_2], 0.0f, 1.0f, Parameter::LINEAR);
    noiseGateCtrl.Init(hw.knob[Terrarium::KNOB_4], -40.0f, 1.0f, Parameter::LINEAR);
    led1.Init(hw.seed.GetPin(Terrarium::LED_1), false);
    led1.Update();
    led2.Init(hw.seed.GetPin(Terrarium::LED_2), false);
    led2.Set(0.0f);
    led2.Update();
    bypass = true;

    hw.StartAdc();
    hw.StartAudio(callback);
}
