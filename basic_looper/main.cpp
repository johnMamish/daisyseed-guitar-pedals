#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"

#include "./debounce.h"

using namespace daisy;
using namespace daisysp;
using namespace terrarium;

// Declare a local daisy_petal for hardware access
DaisyPetal hw;

Parameter volume_in, compression_ratio, compression_threshold;
bool      bypass;
ReverbSc  verb;
Oscillator lfo;

Led led1, led2;


// Length of the loop buffer in samples
#define BUFF_LEN 48000 * 120
static float DSY_SDRAM_BSS base_buff[BUFF_LEN] = { 0 };

#define AUDIO_BLOCKSIZE 16

// State management for looper application
typedef enum record_state_e {
    RECORD_STATE_IDLE,
    RECORD_STATE_INITIAL_LOOP,
    RECORD_STATE_OVERDUB,
    RECORD_STATE_PLAYBACK
} record_state_e;

typedef struct loop_state {
    record_state_e state;
    int32_t size;
    int32_t head;

    int32_t record_buffer_index;
    int32_t modify_buffer_index;
} loop_state_t;

loop_state_t loop_state = {.state = RECORD_STATE_IDLE, .size = -1, .head = 0};

// compression
Compressor input_compressor;

// expected bounce length: 500 audio samples = ~10ms
Debouncer fs1_debounce(500);
Debouncer fs2_debounce(500);

// This runs at a fixed rate, to prepare audio samples
void callback(const float *in, float *out, unsigned int size)
{
    unsigned int size_mono = size / 2;

    hw.ProcessAllControls();

    // debounce footswitches and advance state according to pedal
    fs1_debounce.update(AUDIO_BLOCKSIZE/2, hw.switches[Terrarium::FOOTSWITCH_1].RawState());
    fs2_debounce.update(AUDIO_BLOCKSIZE/2, hw.switches[Terrarium::FOOTSWITCH_2].RawState());
    if (fs1_debounce.rising_edge()) {
        if (loop_state.state == RECORD_STATE_IDLE) {
            loop_state.state = RECORD_STATE_INITIAL_LOOP;
            loop_state.size = 0;
            loop_state.head = 0;
        } else if (loop_state.state == RECORD_STATE_INITIAL_LOOP) {
            loop_state.state = RECORD_STATE_OVERDUB;
            loop_state.head = 0;
        } else if (loop_state.state == RECORD_STATE_OVERDUB) {
            loop_state.state = RECORD_STATE_PLAYBACK;
        } else if (loop_state.state == RECORD_STATE_PLAYBACK) {
            loop_state.state = RECORD_STATE_OVERDUB;
        }
    }

    if ((fs1_debounce.get() == true) && (fs1_debounce.time_in_current_state() > 15000)) {
        loop_state.state = RECORD_STATE_IDLE;
    }

    // set LED according to state
    if (loop_state.state == RECORD_STATE_IDLE) {
        led1.Set(0.0f);
        led2.Set(0.0f);
    } else if (loop_state.state == RECORD_STATE_INITIAL_LOOP) {
        led1.Set(0.0f);
        led2.Set(1.0f);
    } else if (loop_state.state == RECORD_STATE_OVERDUB) {
        led1.Set(1.0f);
        led2.Set(1.0f);
    } else if (loop_state.state == RECORD_STATE_PLAYBACK) {
        led1.Set(1.0f);
        led2.Set(0.0f);
    }

    // apply volume and compression directly to input
    const float vol = volume_in.Process();
    static float input_preprocessed[AUDIO_BLOCKSIZE];
    input_compressor.SetRatio(compression_ratio.Process());
    input_compressor.SetThreshold(compression_threshold.Process());
    for (int i = 0; i < AUDIO_BLOCKSIZE; i++) {
        input_preprocessed[i] = input_compressor.Process(in[i * 2] * vol);
    }

    // update state of loop machinery
    if (loop_state.state == RECORD_STATE_IDLE) {
        for (size_t i = 0; i < size; i += 2) {
            out[i] = input_preprocessed[i/2];
            out[i + 1] = input_preprocessed[i/2];
        }
    } else if (loop_state.state == RECORD_STATE_INITIAL_LOOP) {
        // write the input into the stored buffer. keep track of the size of the loop buffer.
        for (size_t i = 0; i < size; i += 2) {
            out[i] = input_preprocessed[i/2];
            out[i + 1] = input_preprocessed[i/2];

            base_buff[loop_state.size] = input_preprocessed[i/2];

            loop_state.size++;
        }
    } else if (loop_state.state == RECORD_STATE_OVERDUB) {
        // overlay the stored buffer on the output, then accumulate the input into the memory.
        for (size_t i = 0; i < size; i += 2) {
            out[i] = input_preprocessed[i/2] + base_buff[loop_state.head];
            out[i + 1] = input_preprocessed[i/2] + base_buff[loop_state.head];

            base_buff[loop_state.head] += input_preprocessed[i/2];

            loop_state.head++;
            if (loop_state.head == loop_state.size) loop_state.head = 0;
        }
    } else if (loop_state.state == RECORD_STATE_PLAYBACK) {
        // just overlay the stored buffer on the output.

        for (size_t i = 0; i < size; i += 2) {
            out[i] = input_preprocessed[i/2] + base_buff[loop_state.head];
            out[i + 1] = input_preprocessed[i/2] + base_buff[loop_state.head];

            loop_state.head++;
            if (loop_state.head == loop_state.size) loop_state.head = 0;
        }
    }



    led1.Update();
    led2.Update();
}

int main(void)
{
    float samplerate;

    hw.Init();
    samplerate = hw.AudioSampleRate();
    hw.SetAudioBlockSize(AUDIO_BLOCKSIZE);

    // knobs
    volume_in.Init(hw.knob[Terrarium::KNOB_1], 0.f, 1.f, Parameter::LINEAR);
    compression_ratio.Init(hw.knob[Terrarium::KNOB_2], 1.f, 40.f, Parameter::LINEAR);
    compression_threshold.Init(hw.knob[Terrarium::KNOB_2], 0.f, -80.f, Parameter::EXPONENTIAL);

    led1.Init(hw.seed.GetPin(Terrarium::LED_1), false);
    led2.Init(hw.seed.GetPin(Terrarium::LED_2), false);
    led1.Update();
    led2.Update();
    bypass = true;

    hw.StartAdc();
    hw.StartAudio(callback);

    while(1) {

    }
}
