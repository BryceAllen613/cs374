/* calcPi.c calculates PI using the integral of the unit circle.
 * Since the area of the unit circle is PI, 
 *  PI = 4 * the area of a quarter of the unit circle 
 *    (i.e., its integral from 0.0 to 1.0)
 *
 * Joel Adams, Fall 2013 for CS 374 at Calvin College.
 * 
 * Edited to run parallel on MPI by Bryce Allen, 
 * cs374, proj05, Calvin University 10/19/19
 */

#include "integral.h" // integrate()
#include <stdio.h>    // printf(), etc.
#include <stdlib.h>   // exit()
#include <math.h>     // sqrt()
#include <mpi.h>

/* function for unit circle (x^2 + y^2 = 1)
 * parameter: x, a double
 * return: sqrt(1 - x^2)
 */
double f(double x)
{
   return sqrt(1.0 - x * x);
}

/* retrieve desired number of trapezoids from commandline arguments
 * parameters: argc: the argument count
 *             argv: the argument vector
 * return: the number of trapezoids to be used.
 */
unsigned long long processCommandLine(int argc, char **argv)
{
   if (argc == 1)
   {
      return 1;
   }
   else if (argc == 2)
   {
      //       return atoi( argv[1] );
      return strtoull(argv[1], 0, 10);
   }
   else
   {
      fprintf(stderr, "Usage: ./calcPI [numTrapezoids]");
      exit(1);
   }
}

int main(int argc, char **argv)
{
   //Setup
   const long double REFERENCE_PI = 3.141592653589793238462643383279L;
   int numProcesses = 0;
   double startTime, endTime = 0;
   int id = -1;
   long double approximatePI = 0;
   long double total;

   //Initilize MPI
   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &id);
   MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
   
   //Setup number of trapezoids per process
   unsigned long long numTrapezoids = processCommandLine(argc, argv)/numProcesses;

   //Setup range to compute per process.
   double x = numProcesses;
   double chunkSize = 1.0/x;
   
   //Start Timing
   startTime = MPI_Wtime();
   
   //Calculate integral of unit circle range 0 to 1, evenly distributed among pe's.
   approximatePI = integrateTrap(chunkSize*id, chunkSize*(id+1),
      numTrapezoids);
   //reduce to a global long double 'total'
   MPI_Reduce(&approximatePI, &total, 1, MPI_LONG_DOUBLE, MPI_SUM, 
            0, MPI_COMM_WORLD);

   total *= 4.0;

   //End Timing
   endTime = MPI_Wtime() - startTime;

   //process 0 outputs results.
   if(id == 0)
   {
      printf("Using %d processes and %llu trapezoids, the approximate vs actual values of PI are:\n%.30Lf\n%.30Lf\n",
          numProcesses, (numTrapezoids*numProcesses), total, REFERENCE_PI);
      printf("\nTime: %f\n", endTime);
   }
   MPI_Finalize();
   return 0;
}
