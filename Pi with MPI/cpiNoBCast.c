#include "mpi.h"
#include <stdio.h>
#include <math.h>

int main( int argc, char *argv[])
{
    int n, i;
    double PI25DT = 3.141592653589793238462643;
    double pi, h, sum, x;

    int numprocs, myid;
    double startTime, endTime;

    /* Initialize MPI and get number of processes and my number or rank*/
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);
    MPI_Status status;

    /* Processor zero sets the number of intervals and starts its clock*/
    if (myid==0)
    {
       n=8;
       startTime=MPI_Wtime();
    }
    /* Use MPI_Send and MPI_Recv to share number of intervals to all processes */
    if (myid == 0 ) {
        for (i = 1; i < numprocs; i = i + 1) {
            MPI_Send(&n, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    }

    if (myid != 0) {
        MPI_Recv(&n, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    }

    /* Calculate the width of intervals */
    h   = 1.0 / (double) n;

    /* Initialize sum */
    sum = 0.0;
    /* Step over each inteval I own */
    for (i = myid+1; i <= n; i += numprocs)
    {
            /* Calculate midpoint of interval */
            x = h * ((double)i - 0.5);
            /* Add rectangle's area = height*width = f(x)*h */
            sum += (4.0/(1.0+x*x))*h;
    }
    /* Get sum total on processor 0 using MPI_Send and MPI_Recv*/
    if (myid != 0 ) {
        MPI_Send(&sum, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }

    /*Use an array to collect results and print the approximate value of pi and runtime*/
    double sumArray[numprocs - 1];
    if (myid == 0 ) {
        for (i = 1; i < numprocs; i += 1) {
            MPI_Recv(&sumArray[i-1], 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status);
        }

        for (i = 0; i < numprocs - 1; i += 1) {
           sum += sumArray[i];
        }


       printf("pi is approximately %.16f, Error is %e\n",
                      sum, fabs(sum - PI25DT));
       endTime=MPI_Wtime();
       printf("runtime is=%.16f",endTime-startTime);
    }
    MPI_Finalize();
    return 0;
}
