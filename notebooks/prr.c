#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>  // Use standard timing

#define NX 500       // Number of grid points in x-direction
#define NY 500       // Number of grid points in y-direction
#define NT 1000      // Number of time steps
#define DX 0.01      // Grid spacing in x-direction
#define DY 0.01      // Grid spacing in y-direction
#define DT 0.001     // Time step size
#define C 1.0        // Wave speed
#define C2 (C * C)   // Precomputed constant for efficiency

void initialize(double u[NX][NY], double u_prev[NX][NY]) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < NX; i++) {
        for (int j = 0; j < NY; j++) {
            u[i][j] = 0.0;        // Initial displacement
            u_prev[i][j] = 0.0;   // Previous time step
        }
    }
    int cx = NX / 2;
    int cy = NY / 2;
    u[cx][cy] = 1.0;  // Introduce disturbance at the center
}

void wave_solver(double u[NX][NY], double u_prev[NX][NY]) {
    double u_next[NX][NY];

    for (int t = 0; t < NT; t++) {
        // Parallelize computation of u_next
        #pragma omp parallel for collapse(2)
        for (int i = 1; i < NX - 1; i++) {
            for (int j = 1; j < NY - 1; j++) {
                double uxx = (u[i+1][j] - 2 * u[i][j] + u[i-1][j]) / (DX * DX);
                double uyy = (u[i][j+1] - 2 * u[i][j] + u[i][j-1]) / (DY * DY);
                u_next[i][j] = 2 * u[i][j] - u_prev[i][j] + (C2 * DT * DT) * (uxx + uyy);
            }
        }

        // Parallelize array updates
        #pragma omp parallel for collapse(2)
        for (int i = 0; i < NX; i++) {
            for (int j = 0; j < NY; j++) {
                u_prev[i][j] = u[i][j];
                u[i][j] = u_next[i][j];
            }
        }
    }
}

int main() {
    double u[NX][NY], u_prev[NX][NY];

    initialize(u, u_prev);

    // Measure execution time using clock()
    clock_t start_time = clock();
    wave_solver(u, u_prev);
    clock_t end_time = clock();

    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Execution Time: %f seconds\n", elapsed_time);

    return 0;
}
