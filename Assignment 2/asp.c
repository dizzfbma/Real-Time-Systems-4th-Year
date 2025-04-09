/* asp.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int compare(const void *a, const void *b) {
    int int_a = *((int*)a);
    int int_b = *((int*)b);
    // Return -1 if int_a < int_b, 1 if int_a > int_b, and 0 if equal.
    return (int_a > int_b) - (int_a < int_b);
}

int main() {
    size_t array_size = 10000000;  // Adjust this value to change data intensity
    int *array = malloc(array_size * sizeof(int));
    if (array == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));  // Seed the random number generator

    clock_t start, gen_end, sort_end;
    start = clock();

    // Generate an array of random integers
    for (size_t i = 0; i < array_size; i++) {
        array[i] = rand();
    }
    gen_end = clock();
    printf("Generated array of size %zu in %f seconds.\n", array_size, (double)(gen_end - start) / CLOCKS_PER_SEC);

    // Sort the array using qsort
    qsort(array, array_size, sizeof(int), compare);
    sort_end = clock();
    printf("Sorted the array in %f seconds.\n", (double)(sort_end - gen_end) / CLOCKS_PER_SEC);

    free(array);
    return 0;
}

