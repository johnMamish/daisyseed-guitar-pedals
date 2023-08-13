#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"

using namespace daisy;
using namespace daisysp;
using namespace terrarium;

// Declare a local daisy_petal for hardware access
DaisyPetal hw;
bool bypass;

// Parameter

Led led1;

void callback(const float *in, float *out, unsigned int size)
{
    hw.ProcessAllControls();
    led1.Update();

    if (hw.switches[Terrarium::FOOTSWITCH_1].RisingEdge())
    {
        bypass = !bypass;
        led1.Set(bypass ? 0.0f : 1.0f);
    }

    for (size_t i = 0; i < size; i += 2)
    {
        if (bypass)
        {
            out[i] = in[i];         // left
            out[i + 1] = in[i + 1]; // right
        }
        else
        {
            out[i] = sqrt(4 * in[i]) / 2;
            out[i + 1] = sqrt(4 * in[i + 1]) / 2;
        }
    }
}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(12);

    // vtime.Init(hw.knob[Terrarium::KNOB_1], 0.6f, 0.999f, Parameter::LOGARITHMIC);
    // vfreq.Init(hw.knob[Terrarium::KNOB_2], 500.0f, 20000.0f, Parameter::LOGARITHMIC);
    // vsend.Init(hw.knob[Terrarium::KNOB_3], 0.0f, 1.0f, Parameter::LINEAR);
    // lfo_speed.Init(hw.knob[Terrarium::KNOB_4], 0.005f, 0.15f, Parameter::LOGARITHMIC);
    // amplitude.Init(hw.knob[Terrarium::KNOB_5], 0.65f, 0.999f, Parameter::LINEAR);
    // verb.Init(samplerate);

    // lfo.Init(samplerate);

    led1.Init(hw.seed.GetPin(Terrarium::LED_1),false);
    // led2.Init(hw.seed.GetPin(Terrarium::LED_2),false, 10000.0f);
    led1.Update();
    bypass = true;

    hw.StartAdc();
    hw.StartAudio(callback);
}
