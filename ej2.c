#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size, len;
    char name[MPI_MAX_PROCESSOR_NAME];
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Get_processor_name(name, &len);

    int N = (argc > 1) ? atoi(argv[1]) : 1000;

    if (N % size != 0) {
        if (rank == 0)
            printf("ERROR: N=%d no divisible entre %d. Usa N múltiplo.\n", N, size);
        MPI_Finalize();
        return 1;
    }

    int filas = N / size;

    double *A = malloc(filas * N * sizeof(double));
    double *B = malloc(N * N * sizeof(double));
    double *C = malloc(filas * N * sizeof(double));

    for (int i = 0; i < filas * N; i++) A[i] = 1.0;
    for (int i = 0; i < N * N; i++)       B[i] = 2.0;

    // ===== MEDIR TIEMPO DE CÁLCULO =====
    MPI_Barrier(MPI_COMM_WORLD);
    double t0 = MPI_Wtime();

    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < N; j++) {
            double suma = 0.0;
            for (int k = 0; k < N; k++)
                suma += A[i * N + k] * B[k * N + j];
            C[i * N + j] = suma;
        }
    }

    double t1 = MPI_Wtime();
    double mi_tiempo = t1 - t0;

    // ===== RECOLECTAR TIEMPOS Y NOMBRES DE NODO =====
    double *todos = NULL;
    int    *todos_rank = NULL;
    char (*nombres)[MPI_MAX_PROCESSOR_NAME] = NULL;

    if (rank == 0) {
        todos      = malloc(size * sizeof(double));
        todos_rank = malloc(size * sizeof(int));
        nombres    = malloc(size * sizeof(*nombres));
    }

    MPI_Gather(&mi_tiempo, 1, MPI_DOUBLE, todos, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Gather(&rank,     1, MPI_INT,    todos_rank, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gather(name, MPI_MAX_PROCESSOR_NAME, MPI_CHAR,
               nombres, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, 0, MPI_COMM_WORLD);

    // ===== CADA RANK REPORTA =====
    printf("[%s] Rank %3d/%d | filas %4d-%4d | tiempo %.4f s\n",
           name, rank, size, rank * filas, (rank + 1) * filas - 1, mi_tiempo);
    fflush(stdout);

    // ===== RANK 0 CONSOLIDA =====
    if (rank == 0) {
        double min_t = todos[0], max_t = todos[0], sum_t = 0;
        int rank_max = 0, rank_min = 0;

        for (int i = 0; i < size; i++) {
            if (todos[i] < min_t) { min_t = todos[i]; rank_min = i; }
            if (todos[i] > max_t) { max_t = todos[i]; rank_max = i; }
            sum_t += todos[i];
        }

        printf("\n");
        printf("╔══════════════════════════════════════════════════════╗\n");
        printf("║  EJERCICIO 2: Matrices %dx%d (%d procesos)      \n", N, N, size);
        printf("╠══════════════════════════════════════════════════════╣\n");
        printf("║  Filas por proceso:       %d\n", filas);
        printf("║  Verificación C[0][0]:    %.2f (esperado %.2f)\n",
               C[0], (double)N * 2.0);
        printf("╠══════════════════════════════════════════════════════╣\n");
        printf("║  TIEMPO POR PROCESO:\n");
        printf("║    Mínimo:  %.4f s (Rank %d en %s)\n",
               min_t, rank_min, nombres[rank_min]);
        printf("║    Máximo:  %.4f s (Rank %d en %s)\n",
               max_t, rank_max, nombres[rank_max]);
        printf("║    Promedio:%.4f s\n", sum_t / size);
        printf("║    Desbalance: %.4f s (%.2f%%)\n",
               max_t - min_t, 100.0 * (max_t - min_t) / max_t);
        printf("╚══════════════════════════════════════════════════════╝\n");

        free(todos); free(todos_rank); free(nombres);
    }

    free(A); free(B); free(C);
    MPI_Finalize();
    return 0;
}