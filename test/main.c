#include "tetromino_test.h"
#include "playfield_test.h"
#include "shuffle_test.h"


int main() {
    test_tetromino_copy_and_rotate();

    test_shuffled_samples_are_within_expected_range();
    test_queue_visibility_and_sampling_triggering_correct_shuffling();
    test_shuffled_sample_index_occurrence_consistency();

    test_empty_playfield_vacancy_top();
    test_empty_playfield_vacancy_bottom();
    test_playfield_tetromino_placement();
    
    print_test_report();
    return 0;
}