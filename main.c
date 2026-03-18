#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LOTTO_MIN 1
#define LOTTO_MAX 45
#define PICK_COUNT 6

static int has_duplicate_numbers(const int nums[PICK_COUNT]) {
    for (int i = 0; i < PICK_COUNT; i++) {
        for (int j = i + 1; j < PICK_COUNT; j++) {
            if (nums[i] == nums[j]) {
                return 1;
            }
        }
    }
    return 0;
}

static int load_draw_data(FILE *fp, int freq[LOTTO_MAX + 1], int last_seen[LOTTO_MAX + 1], int *invalid_lines) {
    char line[256];
    int draw_index = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        int nums[PICK_COUNT];
        int parsed = sscanf(line, "%d %d %d %d %d %d",
            &nums[0], &nums[1], &nums[2], &nums[3], &nums[4], &nums[5]);

        if (parsed != PICK_COUNT || has_duplicate_numbers(nums)) {
            (*invalid_lines)++;
            continue;
        }

        int valid = 1;
        for (int i = 0; i < PICK_COUNT; i++) {
            if (nums[i] < LOTTO_MIN || nums[i] > LOTTO_MAX) {
                valid = 0;
                break;
            }
        }

        if (!valid) {
            (*invalid_lines)++;
            continue;
        }

        for (int i = 0; i < PICK_COUNT; i++) {
            int n = nums[i];
            freq[n]++;
            last_seen[n] = draw_index;
        }

        draw_index++;
    }

    return draw_index;
}

static int compare_int(const void *a, const void *b) {
    int x = *(const int *)a;
    int y = *(const int *)b;
    return x - y;
}

int main(int argc, char *argv[]) {
    FILE *fp = fopen("draws.txt", "r");

    int freq[LOTTO_MAX + 1] = {0};
    int last_seen[LOTTO_MAX + 1];
    double weight[LOTTO_MAX + 1];
    int chosen[LOTTO_MAX + 1] = {0};
    int picks[PICK_COUNT];
    int invalid_lines = 0;
    unsigned int seed = (unsigned int)time(NULL);

    for (int i = LOTTO_MIN; i <= LOTTO_MAX; i++) {
        last_seen[i] = -1;
    }

    if (argc >= 2) {
        char *endptr = NULL;
        unsigned long parsed_seed = strtoul(argv[1], &endptr, 10);
        if (endptr != argv[1] && *endptr == '\0') {
            seed = (unsigned int)parsed_seed;
        } else {
            fprintf(stderr, "Invalid seed '%s'. Expected an unsigned integer.\n", argv[1]);
            if (fp != NULL) {
                fclose(fp);
            }
            return 1;
        }
    }

    int draw_index = 0;
    if (fp != NULL) {
        draw_index = load_draw_data(fp, freq, last_seen, &invalid_lines);
        fclose(fp);
    }

    int total_draws = draw_index;

    for (int i = LOTTO_MIN; i <= LOTTO_MAX; i++) {
        if (total_draws == 0) {
            weight[i] = 1.0;
            continue;
        }

        double frequency_score = (double)freq[i] / (double)total_draws;
        double recency_score = 0.0;
        if (last_seen[i] >= 0) {
            recency_score = (double)(last_seen[i] + 1) / (double)total_draws;
        }

        /* Heuristic weighting from past frequency and recency (no guarantee). */
        weight[i] = 1.0 + (frequency_score * 7.0) + (recency_score * 2.0);
    }

    srand(seed);

    for (int pick_idx = 0; pick_idx < PICK_COUNT; pick_idx++) {
        double total_weight = 0.0;
        for (int n = LOTTO_MIN; n <= LOTTO_MAX; n++) {
            if (!chosen[n]) {
                total_weight += weight[n];
            }
        }

        double r = ((double)rand() / (double)RAND_MAX) * total_weight;
        double cumulative = 0.0;
        int selected = LOTTO_MIN;

        for (int n = LOTTO_MIN; n <= LOTTO_MAX; n++) {
            if (chosen[n]) {
                continue;
            }
            cumulative += weight[n];
            if (r <= cumulative) {
                selected = n;
                break;
            }
        }

        chosen[selected] = 1;
        picks[pick_idx] = selected;
    }

    qsort(picks, PICK_COUNT, sizeof(int), compare_int);

    printf("Recommended lotto numbers (6): ");
    for (int i = 0; i < PICK_COUNT; i++) {
        printf("%d", picks[i]);
        if (i < PICK_COUNT - 1) {
            printf(" ");
        }
    }
    printf("\n");

    if (total_draws == 0) {
        printf("draws.txt not found or empty. Used uniform random selection.\n");
    } else {
        printf("Analyzed %d past draws from draws.txt using weighted heuristic.\n", total_draws);
    }

    if (invalid_lines > 0) {
        printf("Skipped %d invalid line(s) in draws.txt.\n", invalid_lines);
    }

    printf("Seed: %u\n", seed);
    printf("Note: In a fair lottery, all number combinations have equal true probability.\n");

    return 0;
}
