#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024  // Define el tamaño de la matriz

void matrix_multiply(int *A, int *B, int *C, int stripe_size) {
    for (int i = 0; i < stripe_size; i++) {
        for (int j = 0; j < N; j++) {
            C[i * N + j] = 0;
            for (int k = 0; k < N; k++) {
                C[i * N + j] += A[i * N + k] * B[k * N + j];
            }
        }
    }
}

void print_matrix_section(char *label, int *matrix, int rows, int cols) {
    printf("%s:\n", label);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%4d ", matrix[i * cols + j]);
        }
        printf("\n");
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    srand(time(NULL) + rank);  // Make sure different ranks get different seeds
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int stripe_size = N / size;
    int *A = NULL, *B = NULL, *C = NULL, *subA = NULL, *subC = NULL;

    if (rank == 0) {
        A = (int *)malloc(N * N * sizeof(int));
        B = (int *)malloc(N * N * sizeof(int));
        C = (int *)malloc(N * N * sizeof(int));
        // Initialize matrices A and B
        for (int i = 0; i < N * N; i++) {
            A[i] = rand() % 100;
            B[i] = rand() % 100;
        }
    } else {
        B = (int *)malloc(N * N * sizeof(int));  // Ensure B is allocated in all processes
    }

    subA = (int *)malloc(stripe_size * N * sizeof(int));
    subC = (int *)malloc(stripe_size * N * sizeof(int));

    double start_time = MPI_Wtime();

    // Distribute rows of A
    MPI_Scatter(A, stripe_size * N, MPI_INT, subA, stripe_size * N, MPI_INT, 0, MPI_COMM_WORLD);

    // Broadcast B to all processes
    MPI_Bcast(B, N * N, MPI_INT, 0, MPI_COMM_WORLD);

    // Perform matrix multiplication
    matrix_multiply(subA, B, subC, stripe_size);

    

    if (rank == 0) {
        printf("Initial Matrix A\n");
        print_matrix_section("Initial Matrix A", A, N, N);
        print_matrix_section("Initial Matrix B", B, N, N);
    }

    print_matrix_section("Sub-matrix A received at rank", subA, stripe_size, N);
    print_matrix_section("Sub-matrix C computed at rank", subC, stripe_size, N);

    // Gather results in C
    MPI_Gather(subC, stripe_size * N, MPI_INT, C, stripe_size * N, MPI_INT, 0, MPI_COMM_WORLD);

    double end_time = MPI_Wtime();

    if (rank == 0) {
        
        print_matrix_section("Resultant Matrix C", C, N, N);
        printf("Tiempo de cálculo: %f segundos\n", end_time - start_time);
    }

    // Free memory
    free(A);
    free(B);
    free(C);
    free(subA);
    free(subC);

    MPI_Finalize();


}


