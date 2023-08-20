#include <cstdio>

#include "gtest/gtest.h"

#include "NoiseGate.h"

namespace
{
    TEST(NoiseGateTest, InitialState)
    {
        NoiseGate gate;
        gate.Init(44100, -32.0f);
        EXPECT_FALSE(gate.IsOpen());
    }

    TEST(NoiseGateTest, SimplifiedBehavior)
    {
        NoiseGate gate;
        gate.Init(1, -3 /* db, i.e. ~0.5 */, -4 /* db, i.e. ~0.4*/, 4);
        gate.SetAttackTime(0);
        gate.SetReleaseTime(0);
        ASSERT_FALSE(gate.IsOpen());

        // if off, should remain closed
        for (size_t i = 0; i < 8; i++)
        {
            ASSERT_EQ(gate.Process(0), 0.0f);
            ASSERT_FALSE(gate.IsOpen());
        }

        // if turned on a little but not above the on threshold,
        // should remain closed
        for (size_t i = 0; i < 8; i++)
        {
            ASSERT_EQ(gate.Process(0.45), 0.0f);
            ASSERT_FALSE(gate.IsOpen());
        }

        // if turned on, it should remain off...
        for (size_t i = 0; i < 4; i++)
        {
            ASSERT_FALSE(gate.IsOpen());
            ASSERT_EQ(gate.Process(0.55), 0.0f);
        }
        // ...until the hold time has been reached
        ASSERT_TRUE(gate.IsOpen());
        ASSERT_EQ(gate.Process(0.55), 1.0f);
        ASSERT_TRUE(gate.IsOpen());

        // should remain on while above the threshold
        for (size_t i = 0; i < 4; i++)
        {
            ASSERT_EQ(gate.Process(0.55), 1.0f);
            ASSERT_TRUE(gate.IsOpen());
        }

        // if turned down a little but not below the off threshold,
        // should remain open
        for (size_t i = 0; i < 4; i++)
        {
            ASSERT_EQ(gate.Process(0.45), 1.0f);
            ASSERT_TRUE(gate.IsOpen());
        }

        // if turned off, it should remain on...
        for (size_t i = 0; i < 4; i++)
        {
            ASSERT_TRUE(gate.IsOpen());
            ASSERT_EQ(gate.Process(0.35), 1.0f);
        }
        // ...until the hold time has been reached
        ASSERT_FALSE(gate.IsOpen());
        ASSERT_EQ(gate.Process(0.35), 0.0f);
        ASSERT_FALSE(gate.IsOpen());
    }

    TEST(NoiseGateTest, TestSampleFile)
    {
        // noise_test.wav:
        // ~-20db white noise for 10s
        // -1db 440Hz tone at 2s for 4s
        // .raw is the same thing, but no header
        // and 32-bit floats. Should be identical to expected format.
        // TODO: how to make work regardless of if cwd is project root or /utils?
        FILE *f = fopen("samples/noise_test.raw", "rb");
        int sampleRate = 44100;
        size_t totalSamples = 10 /*seconds*/ * sampleRate * 2 /*channels*/;
        float fileBuffer[totalSamples];
        ASSERT_EQ(fread(fileBuffer, sizeof(float), totalSamples, f), totalSamples);

        size_t audioBlockSize = 12;
        float audioBlock[audioBlockSize];
        size_t i = 0;

        NoiseGate gate;
        gate.Init(sampleRate, -7.0f, -10.0f, 0.001f, 0.02f, 0.010f, 0.05f);

        bool gateState = false;
        size_t gateJitter = 0; // count the number of times the gate toggles

        // load the first 1s through the filter
        for (i = 0; i < (1 * sampleRate * 2); i += audioBlockSize)
        {
            gate.ProcessBlock(fileBuffer + i, audioBlock, audioBlockSize);
            if (gate.IsOpen() != gateState)
            {
                gateState = gate.IsOpen();
                gateJitter++;
            }
        }
        // Gate should be closed
        EXPECT_FALSE(gate.IsOpen());
        // it should never have opened
        ASSERT_EQ(gateJitter, 0);

        // load up to 2.5s mark through the filter
        for (; i < (2.5 * sampleRate * 2); i += audioBlockSize)
        {
            gate.ProcessBlock(fileBuffer + i, audioBlock, audioBlockSize);
        }
        EXPECT_TRUE(gate.IsOpen());
        // it should never have opened
        ASSERT_EQ(gateJitter, 0);

        // load up to 5.5s mark through the filter
        for (; i < (5.5 * sampleRate * 2); i += audioBlockSize)
        {
            gate.ProcessBlock(fileBuffer + i, audioBlock, audioBlockSize);
            if (gate.IsOpen() != gateState)
            {
                gateState = gate.IsOpen();
                gateJitter++;
            }
        }
        EXPECT_TRUE(gate.IsOpen());
        // it should have opened only once
        ASSERT_EQ(gateJitter, 1);

        // at the 6 second mark, it should close
        for (; i < (6.5 * sampleRate * 2); i += audioBlockSize)
        {
            gate.ProcessBlock(fileBuffer + i, audioBlock, audioBlockSize);
            if (gate.IsOpen() != gateState)
            {
                gateState = gate.IsOpen();
                gateJitter++;
            }
        }
        EXPECT_FALSE(gate.IsOpen());
        // it should have closed only once
        ASSERT_EQ(gateJitter, 2);

        // load the rest of the data through
        for (; i < (totalSamples / audioBlockSize); i += audioBlockSize)
        {
            gate.ProcessBlock(fileBuffer + i, audioBlock, audioBlockSize);
            if (gate.IsOpen() != gateState)
            {
                gateState = gate.IsOpen();
                gateJitter++;
            }
        }
        EXPECT_FALSE(gate.IsOpen());
        // it should not have opened again
        ASSERT_EQ(gateJitter, 2);
    }
}
