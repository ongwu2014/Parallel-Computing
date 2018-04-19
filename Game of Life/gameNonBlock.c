/* Inclusions */
#include <mpi.h>      /* mpi */
#include <stdlib.h>   /* memory and rand */
#include <stdio.h>    /* prints */

/* Utility function to print populations */
int PrintPop(char *filename, int **matrix, int N)
{
  FILE       *fp;
  char        newFilename[100];

  int ierr,numprocs, myid, Nlocal;
  int row,i,col;

  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  sprintf(newFilename,"%s.%05d",filename,myid);
  Nlocal=N/numprocs;;

  if ((fp = fopen(newFilename, "w")) == NULL)
  {
     printf("Error: can't open output file %s\n", newFilename);
     exit(1);
  }

  for (i=1; i <= Nlocal; i++)
  {
     row =  myid*Nlocal+i;
     for (col=1; col <= N; col++)
     {
       fprintf(fp,"(%i,%i):%i\n", row, col, matrix[i][col]);
     }
     fprintf(fp,"\n");
  }
  fflush(fp);
  fclose(fp);
  return 0;
}


/*---------------------------------------------
 *
 * This code (game.c) runs Conway's game of life
 * on a NumCells by NumCells grid.  The number of
 * generations is controlled by NumGenerations.
 * These can be set at the command line by
 * using -n and -g each followed by the appropriate number.
 *
 * Compile with:
 * mpicc -o game game.c
 *
 * Then to run with 20x20 cells and 100 generations
 * type:
 *
 * mpiexec ./game  -n 20 -g 100
 *
 *
 * DETAILS:
 * Initially the population is set to have a beacon at roughly
 * the middle. This requires NumCells > 5 and means the population
 * should oscillate between 6 and 8.
 * Alternatively a random population can be initialized
 * The percenatge can be changed by -p .2 for 20% cells to be alive.
 *
 * -----------------------------------------------*/
int main(int argc, char* argv[]) {

  /* declarations */
  int **uNew, **uOld;
  int numprocs=1, myid=0;
  int i,stepCounter;
  int row,col,r,c;
  int live;

  /* default values for problem size, number of generations */
  int NumCells=20;
  int NumGenerations=100;
  int DEBUG =0;
  float InitialPop=2.0;
  int arg_index=1;

  /* timers */
  double startTime, endTime;

  /* declarations for optional debug printouts */
  FILE       *fp;
  char        filename[100];


  /* imitialize MPI */
  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD,&myid);

  /* Read optional command line arguments to overide defaults and allow prints*/
  while (arg_index < argc)
  {
      /* Optionally redefine local problem size from command line */
      if ( strcmp(argv[arg_index], "-n") == 0 )
      {
         arg_index++;
         NumCells = atoi(argv[arg_index++]);
      }
      /* Optionally redefine number of generations from command line */
      else if ( strcmp(argv[arg_index], "-g") == 0 )
      {
         arg_index++;
         NumGenerations = atoi(argv[arg_index++]);
      }
      /* Optionally print final population */
      else if ( strcmp(argv[arg_index], "-p") == 0 )
      {
         arg_index++;
         InitialPop = atof(argv[arg_index++]);
      }
      /* Optionally print final population */
      else if ( strcmp(argv[arg_index], "-debug") == 0 )
      {
         arg_index++;
         DEBUG = atoi(argv[arg_index++]);
      }
  }

  /* Determine size of the grid that processor owns */
  int N = NumCells;
  int Nlocal = NumCells/numprocs;

  /* attempt to allocate memory for new and old populations and zero out if successful*/
  if ( (uNew=(int **)calloc((Nlocal+2),sizeof(int*))) == NULL ) {
    printf("Failed memory allocation for uNew");
    return(-1);
  }
  if ( (uOld=(int **)calloc((Nlocal+2),sizeof(int*))) == NULL ) {
    printf("Failed memory allocation for uOld");
    return(-1);
  }
  for (i=0; i < Nlocal+2; i++)
  {
     if ( (uNew[i]=(int *)calloc((N+2),sizeof(int))) == NULL ) {
         printf("Failed memory allocation for uNew");
         return(-1);
     }
     if ( (uOld[i]=(int *)calloc((N+2),sizeof(int))) == NULL ) {
         printf("Failed memory allocation for uOld");
         return(-1);
     }
  }

  /* Setup beacon */
  if (N < 6)
  {
      printf("Requires NumCells > 5");
      return(-1);
  }
  int Bcol = N/2;
  int Brow = N/2;
  int GlobalRow;
  for (row=1; row <= Nlocal; row++)
  {
      GlobalRow =  myid*Nlocal+row;
      if(GlobalRow==(Brow-1)) uOld[row][Bcol-1] = 1;
      if(GlobalRow==(Brow-1)) uOld[row][Bcol] = 1;
      if(GlobalRow==Brow) uOld[row][Bcol-1] = 1;
      if(GlobalRow==(Brow+1)) uOld[row][Bcol+2] = 1;
      if(GlobalRow==(Brow+2)) uOld[row][Bcol+1] = 1;
      if(GlobalRow==(Brow+2)) uOld[row][Bcol+2] = 1;
  }

  /* Initilize population */
  for (row=1; row <= Nlocal; row++)
  {
      for (col=1; col <= N; col++)
      {
         if ((float)rand() > RAND_MAX*InitialPop)
         {
            uOld[row][col] = 1;
         }
      }
  }

  /* start timer for code execution time */
  startTime = MPI_Wtime();

  MPI_Status status1, status2, status3, status4;
  MPI_Request request1, request2, request3, request4;

  /* iteration loop */
  for (stepCounter=0; stepCounter < NumGenerations; stepCounter++)
  {
       /* communicate a strip of cells with neighboring processors */
       /* to avoid deadlock even IDs send then recv, odd IDs recv then send */

       if(myid < numprocs-1) MPI_Isend(uOld[Nlocal],N+2, MPI_INT, myid+1, 0, MPI_COMM_WORLD,&request1);
       if(myid > 0) MPI_Irecv(uOld[0],N+2, MPI_INT, myid-1, 0, MPI_COMM_WORLD,&request2);
       if(myid > 0) MPI_Isend(uOld[1],N+2, MPI_INT, myid-1, 0, MPI_COMM_WORLD,&request3);
       if(myid < numprocs-1) MPI_Irecv(uOld[Nlocal+1],N+2, MPI_INT, myid+1, 0, MPI_COMM_WORLD,&request4);
        /*  loop over cells */
       for (row=2; row < Nlocal; row++)
       {
           for (col=1; col <= N; col++)
           {
             c=col;

             /* compute number of live neighbors */
             live=0;
             /* neighbors in previous row */
             r=row-1;
             live+= uOld[r][(c-1)]+ uOld[r][c]+ uOld[r][(c+1)];
             /* neighbors in current row */
             r=row;
             live+= uOld[r][(c-1)]+ uOld[r][(c+1)];
             /* neighbors in next row */
             r=row+1;
             live+= uOld[r][(c-1)]+ uOld[r][c]+ uOld[r][(c+1)];

             /* apply birth and survival rules */
             if( uOld[row][col]==0 && live == 3)
             {
                 uNew[row][col]=1;
             }
             if( uOld[row][col]==1 && (live == 3 || live ==2))
             {
                 uNew[row][col]=1;
             }
           }
       }

       if(myid < numprocs-1) MPI_Wait(&request1,&status1);
       if(myid > 0) MPI_Wait(&request2,&status2);
       if(myid > 0) MPI_Wait(&request3,&status3);
       if(myid < numprocs-1) MPI_Wait(&request4,&status4);
       int controller = 0;
       for (controller = 0; controller < 2; controller++) {
           row = 1;
           if (controller == 1) row = Nlocal;
           for (col=1; col <= N; col++)
           {
             c=col;

             /* compute number of live neighbors */
             live=0;
             /* neighbors in previous row */
             r=row-1;
             live+= uOld[r][(c-1)]+ uOld[r][c]+ uOld[r][(c+1)];
             /* neighbors in current row */
             r=row;
             live+= uOld[r][(c-1)]+ uOld[r][(c+1)];
             /* neighbors in next row */
             r=row+1;
             live+= uOld[r][(c-1)]+ uOld[r][c]+ uOld[r][(c+1)];

             /* apply birth and survival rules */
             if( uOld[row][col]==0 && live == 3)
             {
                 uNew[row][col]=1;
             }
             if( uOld[row][col]==1 && (live == 3 || live ==2))
             {
                 uNew[row][col]=1;
             }
           }

       }
       /* replace old populations to get ready for next generation */
       for (row=1; row <= Nlocal; row++)
       {
           for (col=1; col <= N; col++)
           {
              uOld[row][col]=uNew[row][col];
              uNew[row][col]=0;
           }
       }
  }

  /* stop timer */
  endTime = MPI_Wtime();

  /* optional printout of Matrix, first check if file is available */
  if (DEBUG != 0)
  {
     sprintf(filename,"ZZZOutputPop");
     PrintPop(filename, uOld, N);
  }

  /* calculate number of live cells in final generation */
  live=0;
  for (row=1; row <= Nlocal; row++)
  {
      for (col=1; col <= N; col++)
      {
         live += uOld[row][col];
      }
  }

  int Total;
  MPI_Reduce(&live, &Total, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

  if( myid == 0)
  {
     live=Total;
     printf("N=%d, P=%d, and generations= %d\n",NumCells,numprocs,NumGenerations);
     printf("     runtime = %.16e\n",endTime-startTime);
     printf("Nlive=%d, PercentLive=%9.6f\n",live, (100.0*live)/(NumCells*NumCells));
  }
  for (i=0; i < Nlocal+2; i++)
  {
     free(uNew[i]);
     free(uOld[i]);
  }

  free(uNew);
  free(uOld);
  MPI_Finalize();

  return 0;

}