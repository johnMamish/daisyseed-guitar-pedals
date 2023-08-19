#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"

using namespace daisy;
using namespace daisysp;
using namespace terrarium;

// Declare a local daisy_petal for hardware access
DaisyPetal hw;
bool bypass;

Parameter blendCtrl;
Led led1;

CrossFade blend;

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
        out[i] = sqrt(in[i]);
        out[i + 1] = sqrt(in[i + 1]);

        out[i] = blend.Process((float &)in[i], out[i]);
        out[i + 1] = blend.Process((float &)in[i + 1], out[i + 1]);
    }
}

void callback(const float *in, float *out, unsigned int size)
{
    hw.ProcessAllControls();
    led1.Update();
    blend.SetPos(blendCtrl.Process());
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
}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(12);
    blend.Init(CROSSFADE_CPOW);

    blendCtrl.Init(hw.knob[Terrarium::KNOB_2], 0.0f, 1.0f, Parameter::LINEAR);
    led1.Init(hw.seed.GetPin(Terrarium::LED_1), false);
    led1.Update();
    bypass = true;

    hw.StartAdc();
    hw.StartAudio(callback);
}
