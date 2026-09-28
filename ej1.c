#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size, len;
    char name[MPI_MAX_PROCESSOR_NAME];
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Get_processor_name(name, &len);

    long long N = (argc > 1) ? atoll(argv[1]) : 1000000000LL;
    double h = 1.0 / (double)N;

    // ===== MEDIR TIEMPO INDIVIDUAL DE CÁLCULO =====
    MPI_Barrier(MPI_COMM_WORLD);
    double t_calc_ini = MPI_Wtime();

    double sum = 0.0;
    long long mi_inicio = (N * rank) / size;
    long long mi_fin    = (N * (rank + 1)) / size;
    long long mis_pasos = mi_fin - mi_inicio;

    for (long long i = mi_inicio; i < mi_fin; i++) {
        double x = (i + 0.5) * h;
        sum += 4.0 / (1.0 + x * x);
    }
    double pi_local = sum * h;

    double t_calc_fin = MPI_Wtime();
    double mi_tiempo_calc = t_calc_fin - t_calc_ini;

    // ===== MEDIR TIEMPO DE REDUCCIÓN =====
    MPI_Barrier(MPI_COMM_WORLD);
    double t_red_ini = MPI_Wtime();

    double pi_total = 0.0;
    MPI_Reduce(&pi_local, &pi_total, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    double t_red_fin = MPI_Wtime();
    double mi_tiempo_red = t_red_fin - t_red_ini;

    // ===== RECOLECTAR TIEMPOS DE TODOS LOS PROCESOS =====
    double *todos_calc = NULL;
    double *todos_red  = NULL;
    if (rank == 0) {
        todos_calc = (double*) malloc(size * sizeof(double));
        todos_red  = (double*) malloc(size * sizeof(double));
    }
    MPI_Gather(&mi_tiempo_calc, 1, MPI_DOUBLE, todos_calc, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Gather(&mi_tiempo_red,  1, MPI_DOUBLE, todos_red,  1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // ===== CADA RANK REPORTA LO QUE HIZO =====
    printf("[%s] Rank %3d/%d | pasos: %10lld | tiempo: %.4f s\n",
           name, rank, size, mis_pasos, mi_tiempo_calc);
    fflush(stdout);

    // ===== RANK 0 CONSOLIDA Y MUESTRA ESTADÍSTICAS =====
    if (rank == 0) {
        double min_calc = todos_calc[0], max_calc = todos_calc[0], sum_calc = 0;
        double min_red  = todos_red[0],  max_red  = todos_red[0],  sum_red  = 0;

        for (int i = 0; i < size; i++) {
            if (todos_calc[i] < min_calc) min_calc = todos_calc[i];
            if (todos_calc[i] > max_calc) max_calc = todos_calc[i];
            sum_calc += todos_calc[i];

            if (todos_red[i] < min_red) min_red = todos_red[i];
            if (todos_red[i] > max_red) max_red = todos_red[i];
            sum_red += todos_red[i];
        }

        printf("\n");
        printf("╔══════════════════════════════════════════════════════╗\n");
        printf("║  EJERCICIO 1: Cálculo de π (N=%lld)              \n", N);
        printf("╠══════════════════════════════════════════════════════╣\n");
        printf("║  Procesos totales:        %d\n", size);
        printf("║  Pi calculado:            %.15f\n", pi_total);
        printf("║  Error vs real:           %.4e\n", pi_total - 3.141592653589793);
        printf("╠══════════════════════════════════════════════════════╣\n");
        printf("║  TIEMPO DE CÁLCULO (por proceso):\n");
        printf("║    Mínimo:                %.4f s\n", min_calc);
        printf("║    Máximo:                %.4f s  ← cuello de botella\n", max_calc);
        printf("║    Promedio:              %.4f s\n", sum_calc / size);
        printf("║    Desbalance:            %.4f s\n", max_calc - min_calc);
        printf("╠══════════════════════════════════════════════════════╣\n");
        printf("║  TIEMPO DE REDUCCIÓN (red):\n");
        printf("║    Mínimo:                %.6f s\n", min_red);
        printf("║    Máximo:                %.6f s\n", max_red);
        printf("║    Promedio:              %.6f s\n", sum_red / size);
        printf("╚══════════════════════════════════════════════════════╝\n");

        free(todos_calc);
        free(todos_red);
    }

    MPI_Finalize();
    return 0;
}