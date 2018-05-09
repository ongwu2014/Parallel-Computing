#include <mpi.h>
#include <math.h>
#include <stdio.h>

#define MIN(a, b) ((a)<(b)?(a):(b))


//#define BLOCK_LOW(id,p,n) ((id)*(n)/(p))
//#define BLOCK_HIGH(id,p,n) (BLOCK_LOW((id)+1,p,n) - 1)
//#define BLOCK_SIZE(id,p,n) (BLOCK_LOW((id)+1)-BLOCK_LOW(id))
//#define BLOCK_OWNER(index,p,n) (((p)*((index)+1)-1)/(n))

int BLOCK_LOW (int id, int p, int n) {
    return id*n/p;
}

int BLOCK_HIGH (int id, int p, int n) {
    return BLOCK_LOW(id+1, p, n) - 1;
}

int BLOCK_SIZE (int id, int p, int n) {
    return BLOCK_LOW(id+1, p, n) - BLOCK_LOW(id, p, n);
}

int BLOCK_OWNER (int index, int p, int n) {
    return p*((index+1)-1)/n;
}

int main (int argc, char *argv[]) {
    int count;
    double elapsedTime;
    int first;
    int globalCount;
    int highValue;
    int i, id;
    int index;
    int lowValue;
    char *marked;
    int n;
    int p;
    int procZeroSize;
    int prime;
    int size;
    //variables for optimization
    int primeSize;
    char *primeMarked;
    int *primeSet;
    int primeSetCounter;
    int primeSetSize;

    MPI_Init(&argc, &argv);


    MPI_Barrier(MPI_COMM_WORLD);


    MPI_Comm_rank (MPI_COMM_WORLD, &id);
    MPI_Comm_size (MPI_COMM_WORLD, &p);

    if (argc != 2) {
        if (!id) {
            printf("Command Line: %s <m>\n", argv[0]);
            MPI_Finalize();
            exit(1);
        }
    }

    n = atoi(argv[1]);

    //add code for optimization.
    primeSize = ceil(sqrt((double)n));
    primeMarked = (char *)malloc(primeSize);
    primeMarked[0] = 1; primeMarked[1] = 1;

    for (i = 2; i <= primeSize; i++) {
        primeMarked[i] = 0;
    }

    int primeK = 2;

    do {
        int base = primeK * primeK;
        for (i = base; i< primeSize; i+=primeK) {
            primeMarked[i] = 1;
        }
        while(primeMarked[++primeK]);
    } while (primeK *primeK <= primeSize);
    primeSetSize = primeSize;
    primeSet = (int *)malloc(primeSetSize * sizeof(int));
    for (i = 0; i < primeSetSize; i++) {
        primeSet[i] = 0;
    }

    primeSetCounter = 0;
    for (i = 3; i <= primeSize; i++) {
        if (primeMarked[i] == 0){
            primeSet[primeSetCounter] = i;
            primeSetCounter++;
        }
    }

    //end first segment
    //printf("Reached Here \n");
    lowValue = 2 + BLOCK_LOW(id,p,n-1);
    highValue = 2 + BLOCK_HIGH(id,p,n-1);


    size = BLOCK_SIZE(id,p,n-1);
    procZeroSize = (n-1)/p;
//printf("Reached Here \n");

    if ((2 + procZeroSize) < (int) sqrt((double)n)) {
        if (!id) printf ("Too many processes. Reduce number of process or increase problem size and try again \n");
        MPI_Finalize();
        exit(1);
    }

    marked = (char*)malloc(size);
    //printf("Reached Here \n");
    elapsedTime = MPI_Wtime();
    if (marked == NULL) {
        printf("Memory allocation failed\n");
        MPI_Finalize();
        exit(1);
    }

    for (i = 0; i < size; i++) {
        marked[i] = 0;
    }

    if (!id) {
        index = 0;
    }
        //printf("Reached Here \n");
    prime = 2;

    int primeIndex = 0;
    do {
        if (prime * prime > lowValue) {
            first = prime * prime - lowValue;
        }
        else {
            if (!(lowValue % prime)) {
                first = 0;
            }
            else {
                first = prime - (lowValue % prime);
            }
        }
        for (i = first; i < size; i += prime) {
            marked[i] = 1;
        }

        if (!id) {
            while (marked[++index]);
            prime = index + 2;
        }
        //printf("Reached Here \n");
        prime = primeSet[primeIndex];
        primeIndex++;
    } while (prime * prime <= n && primeSet[primeIndex-1] != 0);

    count = 0;
    for (i = 0; i < size; i++) {
        if (!marked[i]) {
            count++;
            //debug print

        }
    }

    MPI_Reduce (&count, &globalCount, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);


    elapsedTime = MPI_Wtime() - elapsedTime;



    if (!id) {
        printf("%d primes are less than or equal to %d\n", globalCount, n);
        printf("Total elapsed time: %10.10f\n", elapsedTime);
    }

    MPI_Finalize();
    return 0;
}