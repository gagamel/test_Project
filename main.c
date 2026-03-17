#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LOTTO_MIN 1
#define LOTTO_MAX 45
#define PICK_COUNT 6

static int compare_int(const void *a, const void *b) {
    int x = *(const int *)a;
    int y = *(const int *)b;
    return x - y;
}

int main(void) {
    FILE *fp = fopen("draws.txt", "r");

    int freq[LOTTO_MAX + 1] = {0};
    int last_seen[LOTTO_MAX + 1];
    double weight[LOTTO_MAX + 1];
    int chosen[LOTTO_MAX + 1] = {0};
    int picks[PICK_COUNT];

    for (int i = LOTTO_MIN; i <= LOTTO_MAX; i++) {
        last_seen[i] = -1;
    }

    int draw_index = 0;
    if (fp != NULL) {
        int n1, n2, n3, n4, n5, n6;
        while (fscanf(fp, "%d %d %d %d %d %d", &n1, &n2, &n3, &n4, &n5, &n6) == 6) {
            int nums[PICK_COUNT] = {n1, n2, n3, n4, n5, n6};
            for (int i = 0; i < PICK_COUNT; i++) {
                int n = nums[i];
                if (n >= LOTTO_MIN && n <= LOTTO_MAX) {
                    freq[n]++;
                    last_seen[n] = draw_index;
                }
            }
            draw_index++;
        }
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

    srand((unsigned int)time(NULL));

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

    printf("Note: In a fair lottery, all number combinations have equal true probability.\n");

    return 0;
}
