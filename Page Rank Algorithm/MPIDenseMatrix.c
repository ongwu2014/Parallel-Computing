#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    //Declarations
    double dampingFactor = 0.15;
    int numprocs, myid, i, j, k, K;
    int numPage = 1600;
    int totalSize = numPage * numPage;
    int *nAlloc;
    double *sArray, *pageRank, *yArray, *globalPageRank;

    //Initilize MPI variables
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Status status;

    //nAlloc hold the local row size
    double startTime, endTime;
    K = 1000;
    nAlloc = (int*)malloc(numprocs*sizeof(int));

    //find partion positions
    int minPartition = numPage/numprocs;
    for (i = 0; i < numprocs; i++)
        nAlloc[i] = minPartition;

    int remainder = numPage%numprocs;
    for(i = 0; i < remainder; i++)
        nAlloc[i] += 1;

    //setup array space in each processor
    sArray = (double*)malloc(nAlloc[myid]*numPage*sizeof(double));
    yArray = (double*)malloc(nAlloc[myid]*sizeof(double));
    pageRank = (double*)malloc(nAlloc[myid]*sizeof(double));
    globalPageRank = (double*)malloc(numPage*sizeof(double));

    //initialize Array Spaces
    for(i = 0; i < nAlloc[myid]; i++) {
        for(j = 0; j < numPage; j++) {
            sArray[i*numPage+j] = 0.0;
        }
        pageRank[i] = 1/(double)numPage;
    }

    //Fill sArray with page navigation probabilities
    for (i = 0; i < nAlloc[myid]; i++) {
        if (myid == 0 && i == 0) {
            sArray[1] = 0.5;
            sArray[numPage-1] = 0.5;
            sArray[numPage] = 1.0;
            sArray[numPage+2] = 0.5;
            i++;
        }
        else if (myid == (numprocs-1) && i == (nAlloc[myid]-1)) {
            sArray[i*numPage+numPage-2] = 0.5;
        }
        else {
            int globalRow = myid*nAlloc[myid]+i;
            sArray[i*numPage+globalRow-1] = 0.5;
            sArray[i*numPage+globalRow+1] = 0.5;
        }
    }

    //Apply damping factor on sArray
    for (i = 0; i < nAlloc[myid]; i++) {
        for (j = 0; j < numPage; j++) {
            sArray[i*numPage+j] = ((1-dampingFactor)*sArray[i*numPage+j]) + (dampingFactor/numPage);
        }
    }

    //Start time and perform MatVec K-times
    startTime = MPI_Wtime();
    for (k = 0; k < K; k++) {
        MPI_Allgather(pageRank, nAlloc[myid], MPI_DOUBLE, globalPageRank, nAlloc[myid], MPI_DOUBLE, MPI_COMM_WORLD);
        for (i = 0; i < nAlloc[myid]; i++) {
            yArray[i] = 0.0;
            for(j = 0; j < numPage; j++) {
                yArray[i] += sArray[i*numPage+j]*globalPageRank[j];
            }
        }
        for (i = 0; i < nAlloc[myid]; i++) {
            pageRank[i] = yArray[i];
        }
    }
    endTime = MPI_Wtime();

    //Gather the last calculations
    MPI_Allgather(pageRank, nAlloc[myid], MPI_DOUBLE, globalPageRank, nAlloc[myid], MPI_DOUBLE, MPI_COMM_WORLD);

    //Print the Pageranks using Processor 0
    if (myid == 0) {
        if (numPage < 20) {
            for (i = 0; i < numPage; i++) {
                printf("PR(%d) = %f \n", i, globalPageRank[i]);
            }
        }
        else {
            double max, min;
            max = globalPageRank[0];
            min = globalPageRank[0];
            double sum = 0;
            for (i = 0; i < numPage; i++) {
                if (max < globalPageRank[i])
                    max = globalPageRank[i];
                if (min > globalPageRank[i])
                    min = globalPageRank[i];
                sum+=globalPageRank[i];
            }

            printf("Min Pagerank = %f \n", min);
            printf("Max Pagerank = %f \n", max);
            printf("SUM: %f\n", sum);
        }
        //print the runtime
        printf("RUNTIME = %.16f\n", endTime - startTime);
    }
    MPI_Finalize();
    return 0;
}