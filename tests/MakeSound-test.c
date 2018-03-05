/*
 *  Copyright (c) 2018 Yauheni Kaliuta
 */

/*
 * This will test only one internal function so it must by synced manually
 */

#include <cgreen/cgreen.h>
#include <stdint.h>
#include <string.h>

#define NAME "MakeSound"

static void setup(void)
{
}

static void teardown(void)
{
}

/* #define EMUCLOCK 1 */
/* #define SOUNDFREQ 3 */
/* #define TIMERCLOCK 10 */
#define EMUCLOCK 50
#define SOUNDFREQ (44100/2)
#define TIMERCLOCK 2000000
#define AUDIO_BUFFER_SIZE (SOUNDFREQ / EMUCLOCK)
#define TICKS_PER_EMUCLOCK (TIMERCLOCK / EMUCLOCK)

static int MuteFlag;

static inline void LOCK_OUT() {};
static inline void UNLOCK_OUT() {};

/* This should produce 1s number of ticks 1, then 0 */
static unsigned produced_ticks;

static bool SHIFT_OUT(int *v)
{
	static int i;

	if (i < TICKS_PER_EMUCLOCK) {
		*v = 1;
	} else {
		*v = 0;
	}
	produced_ticks++;
	return true;
}

void MakeSound(uint8_t *p, unsigned len)
{
    int i;
    int j;
    unsigned timer_freq = TIMERCLOCK;
    unsigned ticks_per_sample = timer_freq / SOUNDFREQ;
    unsigned reminder = timer_freq % SOUNDFREQ;
    static unsigned left_numerator = 0;
    static unsigned left_denominator = SOUNDFREQ;
    int tickval;
    int sum;
    int sample_size = sizeof(uint8_t);

    if (MuteFlag)
        goto flush;

    LOCK_OUT();

    for (i = 0; i < len; i++) {
        sum = 0;

        for (j = 0; j < ticks_per_sample; j++) {
            if (!SHIFT_OUT(&tickval))
                goto flush;
            sum += tickval;
        }

        left_numerator += reminder;

        if (left_numerator / left_denominator >= 1) {
            if (!SHIFT_OUT(&tickval))
                goto flush;
            sum += tickval;
            left_numerator -= left_denominator;
        }

        p[i] = sum;
    }

    UNLOCK_OUT();
    return;

flush:
    memset(p, 0, len * sample_size);
}

/* =================== ACTUAL TEST ==================== */

Ensure(consumes_all_ticks_from_one_sec)
{
	uint8_t buf[AUDIO_BUFFER_SIZE];
	int i;

	MakeSound(buf, sizeof(buf));

	assert_that(produced_ticks, is_equal_to(TICKS_PER_EMUCLOCK));
}

TestSuite *my_create_test_suite(void)
{
	TestSuite *suite = create_named_test_suite(NAME);

	set_setup(suite, setup);

	add_test(suite, consumes_all_ticks_from_one_sec);

	set_teardown(suite, teardown);

	return suite;
}
