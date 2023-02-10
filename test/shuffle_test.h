#include <stdlib.h>
#include <string.h>
#include "test.h"
#include "../src/shuffle.h"


void uint8_t_array2string(const uint8_t *arr, char *str, const size_t arr_length)
{
    int q_i = 0, b_i=0;
    while (b_i < arr_length*2) {
        str[b_i++] = arr[q_i++] + '0';
        str[b_i++] = ',';
    }
    str[--b_i] = '\0';
}

bool assert_queues_equal(const uint8_t* expected, const uint8_t* actual, size_t length)
{
    const size_t buffer_size = (length*2);
    char expected_str[buffer_size], actual_str[buffer_size];
    uint8_t_array2string(expected, expected_str, length);
    uint8_t_array2string(actual, actual_str, length);
    return assert(memcmp(expected, actual, length) == 0,
                  "shuffle queue of length %d matched expected state %s (actual: %s)",
                  length, expected_str, actual_str);
}


void test_shuffled_samples_are_within_expected_range()
{ //{{{
    bag_of_7_init(0);  // deterministic shuffling

    bool never_encountered_value_outside_0_to_6_range = true;
    for (int i = 0; i < 10000; ++i) {
        uint8_t sample = bag_of_7_pop_sample();
        if (sample < 0  || sample > 6) {
            never_encountered_value_outside_0_to_6_range = false;
            break;
        }
    }
    assert(never_encountered_value_outside_0_to_6_range,
           "10,000 shuffled samples were within the expected range");
/*}}}*/ }


void test_queue_visibility_and_sampling_triggering_correct_shuffling()
{ //{{{
    bag_of_7_init(1);  // deterministic shuffling

    for (size_t queue_length = 1; queue_length < 15; ++queue_length) {
        uint8_t queue[queue_length]; 
        bag_of_7_write_queue(queue, queue_length);
        assert_queues_equal((const uint8_t[]){0,4,3,1,2,6,5,0,4,6,5,2,3,1}, queue, queue_length);
    }

    uint8_t sample = bag_of_7_pop_sample();

    for (size_t queue_length = 1; queue_length < 7; ++queue_length) {
        uint8_t queue[queue_length]; 
        bag_of_7_write_queue(queue, queue_length);
        assert_queues_equal((const uint8_t[]){4,3,1,2,6,5}, queue, queue_length);
    }

    const size_t queue_length = 6;
    uint8_t queue[queue_length]; 

    const uint8_t expected_samples[] = {
        4,3,1,2,6,5,0,4,6,5,2,3,1,6,2,0,3,4,1,5,5,4,3,0,2,1,6,4,3,5,6,2,1,0,0,4,2,3,5,1,6,3,2,4,6,0,
        5,1,0,3,2,6,5,4,1,1,3,0,5,4,6,2,6,0,2,5,3,4,1,5,3,4,0,1,6,2,5,6,4,3,0,1,2,1,3,0,5,6,4,2,0,2,
        1,6,4,3,5,2,6,0,4,1,5,3,2,6
    };
    const size_t expected_samples_size = sizeof(expected_samples) / sizeof(expected_samples[0]);
    

    for (int i = 0; i < expected_samples_size-queue_length; ++i) {
        sample = bag_of_7_pop_sample();
        assert(sample == expected_samples[i],
               "got expected shuffled queue sample %d (actual: %d)", expected_samples[i], sample);
        bag_of_7_write_queue(queue, queue_length);
        assert_queues_equal(expected_samples+((i+1)*sizeof(expected_samples[0])), queue, queue_length);
    }

/*}}}*/ }


void test_shuffled_sample_index_occurrence_consistency()
{ //{{{
    bag_of_7_init(2);  // deterministic shuffling

    const uint8_t expected_index_occurrences[7] = {2,2,2,2,2,2,2};
    const size_t buffer_size = 14;
    char actual_str[buffer_size], expected_str[buffer_size];
    uint8_t_array2string(expected_index_occurrences, expected_str, 7);

    uint8_t sample;

    const int tests = 10000;
    for (int test = 0; test < tests; ++test) {
        uint8_t index_occurrences[7]={0};


        for (int i = 0; i < 14; ++i) {
            sample = bag_of_7_pop_sample();
            index_occurrences[sample]++;
        }

        if (memcmp(expected_index_occurrences, index_occurrences, 7) != 0) {
            uint8_t_array2string(index_occurrences, actual_str, 7);
            assert(false,
                   "encountered a shuffled index distribution that did not match expected"
                   " distribution %s (actual: %s)",
                   expected_str, actual_str);
            return;
        }
    }

    assert(true,
           "shuffled index distributions matched expected distribution %s over %d tests",
           expected_str, tests);
/*}}}*/ }