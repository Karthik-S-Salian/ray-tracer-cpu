#include <stdio.h>
#include <time.h>

// The function for which you want to measure execution time
void yourFunction() {
    // Your function code goes here
}

int main() {
    // Record the start time
    clock_t start_time = clock();

    // Call the function whose execution time you want to measure
    yourFunction();

    // Record the end time
    clock_t end_time = clock();

    // Calculate the elapsed time in seconds
    double elapsed_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

    // Print the elapsed time
    printf("Time taken: %f seconds\n", elapsed_time);

    return 0;
}
