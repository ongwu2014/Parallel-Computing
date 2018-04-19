#include <omp.h>
#include <stdlib.h>
#include <stdio.h>

int main (int argc, char* argv[]) {

    //Declarations
    double dampingFactor = 0.15;
    int numPage = atoi(argv[1]);
    int totalSize = numPage * numPage;
    double *aArray, *pageRank, *yArray;
    int *iA, *jA;
    aArray = (double*)malloc((2*numPage-1)*sizeof(double));
    iA = (int*)malloc((numPage+1)*sizeof(int));
    jA = (int*)malloc((2*numPage-1)*sizeof(int));
    pageRank = (double*)malloc(numPage*sizeof(double));
    yArray = (double*)malloc(numPage*sizeof(double));
    int i, j, K, k;
    K = 1000;
    double startTime, endTime;

    //setup the sArray
    for (i = 0; i < 2*numPage - 1; i++) {
        if (i == 2 )
            aArray[i] = 1.0;
        else
            aArray[i] = 0.5;
    }

    //setup the iA and initialize pagerank to 1/Numpage
    for (i = 0; i < numPage + 1; i++) {
        if (i == numPage)
            iA[i] = 2*numPage - 1;
        else if (i == 0)
            iA[i] = 0;
        else
            iA[i] = iA[i-1] + 2;
        if (i < numPage)
            pageRank[i] = 1/(double)numPage;
    }

    //setup jA
    jA[0] = 1; jA[1] = numPage -1; jA[2] = 0; jA[3] = 2;
    for (i = 4; i < 2*numPage-1; i++) {
        jA[i] = jA[i -2] + 1;
    }

    //start timer and perform MatVec on the CSR K-times
    startTime = omp_get_wtime();
    for (k = 0; k < K; k++) {
        #pragma omp parallel for private(j)
        for (i = 0; i < numPage; i++) {
            yArray[i] = 0.0;
            for (j = iA[i]; j < iA[i+1]; j++) {
                yArray[i] += aArray[j] * pageRank[jA[j]];
            }
        }
        #pragma omp master
        for (i = 0; i < numPage; i++) {
            //apply damping
            yArray[i] = ((1 - dampingFactor)*yArray[i]) + (dampingFactor / numPage);
            pageRank[i] = yArray[i];
        }
        #pragma end master
    }
    endTime = omp_get_wtime();

    //print pagerank or max and min values
    if (numPage < 20) {
        for (i = 0; i < numPage; i++) {
            printf("PR(%d) = %f \n", i, pageRank[i]);
        }
    }
    else {
        double max, min;
        max = pageRank[0];
        min = pageRank[0];
        for (i = 0; i < numPage; i++) {
            if (max < pageRank[i])
                max = pageRank[i];
            if (min > pageRank[i])
                min = pageRank[i];
        }
        printf("Min Pagerank = %f \n", min);
        printf("Max Pagerank = %f \n", max);
    }

    //print runtime
    printf("RUNTIME = %.16f\n", endTime - startTime);
    return 0;
}