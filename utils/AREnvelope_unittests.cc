#include "gtest/gtest.h"

#include "AREnvelope.h"

namespace
{

    TEST(AREnvelopeTest, InitialState)
    {
        AREnvelope env;
        env.Init(1024, 1, 1);
        EXPECT_EQ(env.GetState(), ARENV_OFF);
        EXPECT_EQ(env.Process(false), 0);
        EXPECT_EQ(env.GetState(), ARENV_OFF);
        EXPECT_EQ(env.Process(true), 0);
        EXPECT_EQ(env.GetState(), ARENV_ATTACK);
    }

    TEST(AREnvelopeTest, Attack)
    {
        AREnvelope env;
        env.Init(1024, 0.015625f, 0 /*16/1024 */);
        env.Process(true); // Trigger state change
        for (size_t i = 0; i < 16; i++)
        {
            ASSERT_EQ(env.GetState(), ARENV_ATTACK);
            ASSERT_EQ(env.Process(true), (i + 1) / 16.0f);
        }
        // one more to turn fully on
        ASSERT_EQ(env.GetState(), ARENV_ATTACK);
        ASSERT_EQ(env.Process(true), 1.0f);
        ASSERT_EQ(env.GetState(), ARENV_ON);

        for (size_t i = 0; i < 16; i++)
        {
            ASSERT_EQ(env.GetState(), ARENV_ON);
            ASSERT_EQ(env.Process(true), 1.0f);
        }
    }

    TEST(AREnvelopeTest, Release)
    {
        AREnvelope env;
        env.Init(1024, 0, 0.015625f /*16/1024 */);
        // Turn on fully
        while (env.GetState() != ARENV_ON)
            env.Process(true);

        // now turn off
        ASSERT_EQ(env.Process(false), 1.0f);
        ASSERT_EQ(env.GetState(), ARENV_RELEASE);

        for (int i = 16; i > 0; i--)
        {
            ASSERT_EQ(env.GetState(), ARENV_RELEASE);
            ASSERT_EQ(env.Process(false), (i - 1) / 16.0f);
        }
        // one more to fully turn off
        ASSERT_EQ(env.GetState(), ARENV_RELEASE);
        ASSERT_EQ(env.Process(false), 0.0f);
        ASSERT_EQ(env.GetState(), ARENV_OFF);
        for (int i = 0; i < 16; i++)
        {
            ASSERT_EQ(env.Process(false), 0.0f);
            ASSERT_EQ(env.GetState(), ARENV_OFF);
        }
    }

    TEST(AREnvelopeTest, SwitchingStateDuringTransition)
    {
        AREnvelope env;
        env.Init(1, 4, 4);

        // get part way into attack mode
        env.Process(true);
        env.Process(true);
        env.Process(true);
        env.Process(true);
        ASSERT_EQ(env.GetState(), ARENV_ATTACK);
        env.Process(false);
        ASSERT_EQ(env.GetState(), ARENV_RELEASE);
        env.Process(true);
        ASSERT_EQ(env.GetState(), ARENV_ATTACK);
        env.Process(true);
        env.Process(true);
        ASSERT_EQ(env.GetState(), ARENV_ON);
    }

    TEST(AREnvelopeTest, ChangingParamsWhileRunning)
    {
        AREnvelope env;
        env.Init(1, 8, 8);

        env.Process(true);
        env.Process(true);
        env.Process(true);
        env.Process(true);
        // now, we should be about half-way through the attack
        ASSERT_EQ(env.GetState(), ARENV_ATTACK);
        // now we set the attack to something lower than the current state
        env.SetAttackTime(2);
        // on the next tick, it should flip to on
        env.Process(true);
        ASSERT_EQ(env.GetState(), ARENV_ON);

        // now same thing for release
        env.Process(false);
        env.Process(false);
        env.Process(false);
        env.Process(false);
        ASSERT_EQ(env.GetState(), ARENV_RELEASE);
        env.SetReleaseTime(2);
        // on the next tick, it should flip to off
        env.Process(false);
        ASSERT_EQ(env.GetState(), ARENV_OFF);
    }

    TEST(AREnvelopeTest, DivideByZeroGuard)
    {
        AREnvelope env;
        env.Init(1, 0, 0);
        for (size_t i = 0; i < 128; i++)
        {
            env.Process(rand() % 2 == 0);
        }
    }
}
