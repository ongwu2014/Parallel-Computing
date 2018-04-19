#include "mpi.h"
#include <stdio.h>
#include <math.h>

int main( int argc, char *argv[])
{
    int n, i;

    int numprocs, myid;
    double startTime, endTime;

    /* Initialize MPI and get number of processes and my number or rank*/
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);
    MPI_Status status;

    double test_Array[10001];
    for (i = 0; i < 10001; i+=1) {
        test_Array[i] = 2.0;
    }
    for (i = 1; i <= 10001; i += 500) {
        startTime=MPI_Wtime();
        int j;
        for (j = 1; j <= 100; j += 1) {
            if (myid == 0) {
                MPI_Send(test_Array, i, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
            }
        if (myid == 1) {
            MPI_Recv(test_Array, i, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);
        }
        if (myid == 1) {
            MPI_Send(test_Array, i, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        }
        if (myid == 0) {
                MPI_Recv(test_Array, i, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, &status);
            }
        }
       endTime=MPI_Wtime();
       if (myid == 0)
       printf("Message size: %i ----- Runtime is=%.16f \n", i, (endTime-startTime)/200);
    }
    MPI_Finalize();
    return 0;
}