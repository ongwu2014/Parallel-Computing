#include <omp.h>
#include <stdlib.h>
#include <stdio.h>

int main (int argc, char* argv[]) {

    //Declarations
    float dampingFactor = 0.15;
    int numPage = atoi(argv[1]);
    int totalSize = numPage * numPage;
    float *sArray, *pageRank, *yArray;
    sArray = (float*)malloc(totalSize*sizeof(float));
    pageRank = (float*)malloc(numPage*sizeof(float));
    yArray = (float*)malloc(numPage*sizeof(float));
    int i, j, K, k;
    K = 1000;
    double startTime, endTime;

    //fill out sArray with 0 and
    //Initial Pagerank vector with 1/Numpage
    for(i = 0; i < numPage; i++) {
        for (j = 0; j < numPage; j++) {
            sArray[i*numPage + j] = 0.0;
        }
        pageRank[i] = 1/(float)numPage;
    }

    //setup sArray with page navigation probabilities
    sArray[1] = 0.5;
    sArray[numPage - 1] = 0.5;
    for (i = 1; i < numPage - 1; i++) {
        for(j = 0; j < numPage; j++) {
            if (i == 1) {
                sArray[i*numPage] = 1.0;
                sArray[i*numPage+2] = 0.5;
                j = numPage;
            }
            else {
                if (j == i) {
                    sArray[(i*numPage) + (j - 1)] = 0.5;
                    sArray[(i*numPage) + (j + 1)] = 0.5;
                    j = numPage;
                }
            }
        }
    }
    sArray[totalSize - 2] = 0.5;

    //Apply damping factor to the sArray
    for (i = 0; i < numPage; i++) {
        for (j = 0; j < numPage; j++) {
            sArray[i*numPage+j] = ((1-dampingFactor)*sArray[i*numPage+j])+(dampingFactor/numPage);
        }
    }

    //start timer and perform MatVec K-times in parallel
    startTime = omp_get_wtime();
    for (k = 0; k < K; k++) {
        #pragma omp parallel for private(j)
        for (i = 0; i < numPage; i++) {
            yArray[i] = 0.0;
            for (j = 0; j < numPage; j++) {
                yArray[i] += sArray[i*numPage+j] * pageRank[j];
            }
        }
        #pragma omp master
        for (i = 0; i < numPage; i++) {
            pageRank[i] = yArray[i];
        }
        #pragma end master
    }
    endTime = omp_get_wtime();

    //Print the Pageranks or max and min values
    if (numPage < 20) {
        for (i = 0; i < numPage; i++) {
            printf("%f \n", pageRank[i]);
        }
    }
    else {
       float max, min;
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
    printf("RUNTIME = %.16f\n", endTime-startTime);
    return 0;
}