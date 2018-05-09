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


    lowValue = 2 + BLOCK_LOW(id,p,n-1);
    highValue = 2 + BLOCK_HIGH(id,p,n-1);


    size = BLOCK_SIZE(id,p,n-1);

    if (lowValue % 2 == 0) {
        if (highValue % 2 == 0) {
            size = (int) floor ((double) size/2.0);
            highValue--;
        } else {
            size = size/2;
        }
        lowValue++;
    } else {
        if (highValue % 2 == 0) {
            size = size/2;
            highValue--;
        } else {
            size = (int) ceil((double)size/2.0);
        }
    }

    procZeroSize = (n-1)/p;

    if ((2 + procZeroSize) < (int) sqrt((double)n)) {
        if (!id) printf ("Too many processes. Reduce number of process or increase problem size and try again \n");
        MPI_Finalize();
        exit(1);
    }


    marked = (char *) malloc (size);

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
        first = 0;
    }

    prime = 3;
    index = 0;

    do {
        if (prime >= lowValue) {
            first = ((prime - lowValue) / 2) + prime;
        } else if (prime * prime > lowValue) {
            first = (prime * prime - lowValue) / 2;
        }
        else {
            if (!(lowValue % prime)) {
                first = 0;
            }
            else {
                first = 1;
                while ((lowValue + (2 * first)) % prime != 0) {
                    first++;
                }
            }
        }
        for (i = first; i < size; i += prime) {
            marked[i] = 1;
        }

        if (!id) {
            while (marked[++index]);
            prime = (3 + (index * 2));
        }
        MPI_Bcast (&prime, 1, MPI_INT, 0, MPI_COMM_WORLD);
    } while (prime * prime <= n);

    if (!id)
        count = 1;
    else
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