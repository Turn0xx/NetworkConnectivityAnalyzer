#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

// for MPI
#include <mpi.h>



// matrix size and some others
#include "graph.h"

// some utilities to print matrices, etc ...
#include "util.h"

#define DIFFTEMPS(a,b) (((b).tv_sec - (a).tv_sec) + ((b).tv_usec - (a).tv_usec)/1000000.)


/* 
 * Transitive closure with the Floyd-Warshall algorithm.
 * Works for directed or undirected graphs.
 * https://fr.wikipedia.org/wiki/Algorithme_de_Warshall
 *
 * @param a : the adjacency matrix of the graph
 * @param c : output: the adjacency matrix resulting from tansitive closure   
 */
void warshall(size_t n, int a[n][n], int c[n][n]) {
#ifdef PROGRESS
    size_t percent=0;
#endif
    //---------------------------------------------------------------------------
    // init matrix c from matrix a
  for (size_t i = 0; i < n; i++)
    for (size_t j = 0; j < n; j++)
      c[i][j] = a[i][j];
/*  updates the value of c[i][j] to true if there is a path 
  *  from i to j that goes through k. */
  for (size_t k = 0; k < n; k++) {
    for (size_t i = 0; i < n; i++) {
      for (size_t j = 0; j < n; j++) {
        c[i][j] = c[i][j] || (c[i][k] && c[k][j]);
      }
    }
#ifdef PROGRESS
  if ((k*100)%n==0) {
    percent++;
    fprintf(stderr, "(%3zu%%)", percent); fflush(stderr);fprintf(stderr, "\b\b\b\b\b\b");
  }
#endif
  }
}

void parallel_warshall(size_t n, int a[n][n], int c[n][n], int num_procs, int rank) {
    // Initialize matrix c from matrix a
    for (size_t i = 0; i < n; i++)
        for (size_t j = 0; j < n; j++)
            c[i][j] = a[i][j];

    // Compute the local transitive closure using the Floyd-Warshall algorithm
    for (size_t k = 0; k < n; k++) {
        // Distribute the k-th row to all processors
        // MPI_Bcast(c[k], n, MPI_INT, k % num_procs, MPI_COMM_WORLD);

        // #pragma omp parallel for schedule(dynamic)
        for (size_t i = rank; i < n; i += num_procs) {
            for (size_t j = 0; j < n; j++) {
                c[i][j] = c[i][j] || (c[i][k] && c[k][j]);
            }
        }
    }
}



/**
 * main
 */
int main (int argc, char **argv) {

 // MPi initialization
  MPI_Init(&argc, &argv);

  int num_procs, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);


	  
	char * filename = 0;
	char * input_type = 0;
	parse_options(argc, argv, &filename, &input_type);

  struct timeval tv_init, tv_begin, tv_end;
  gettimeofday (&tv_init, NULL);


  size_t n = 0;
  // read from an adjacency matrix
  // determine size n of matrix
  if (strcmp(input_type, INPUT_TYPE_ADJ)==0) {
    // first get the adjacency matrix size
    n = matrix_lines_from_file(filename);
    if(rank == 0)
      fprintf(stderr, "* %s has %ld lines.\n", filename, n);
  }
  // read from a file, pairs of 2 nodes per line
  // determine size n of matrix
  if (strcmp(input_type, INPUT_TYPE_PAIRS)==0) {
    // first get the max integers in the pairs
    n = find_max(filename, "\t");
    // assume values in file may start at 0, so add one to max value. 
    n++;
    fprintf(stderr, "* max value found in %s: %ld (about to build a %ldx%ld adjacency matrix)\n", filename, n, n, n);

  }
  // malloc a VLA as a pointer to an array of n integers
  int (*a)[n] = malloc(sizeof(int[n][n]));
  if (a==0) {
    fprintf(stderr, "* could not alloc %ld bytes. Abort.\n", n*n);
    exit(1);
  }

  // Now load, from one type or another
  if (strcmp(input_type, INPUT_TYPE_ADJ)==0) {
    // read data from a file containing an adjacency matrix
    matrix_from_adj_file(filename, n, a);
  }
  // read from a file, pairs of 2 nodes per line
  if (strcmp(input_type, INPUT_TYPE_PAIRS)==0) {
    // read data from a file containing a list of pairs
    // : adjust the separator "\t", ",", ... depending on your file
    matrix_from_pairs_file(filename, n, a, "\t");
  }

  int (*c)[n] = malloc(sizeof(int[n][n]));
  if (c==0) {
    fprintf(stderr, "* could not alloc %ld bytes. Abort.\n", n*n);
    exit(1);
  }

#ifdef DEBUG
  if(rank == 0)
    print_matrix ("a_orig", n , a, 0, n, 0, n, 0, false);
#endif
  //Compute
  if(rank == 0)
    fprintf(stderr, "* starting computation (n=%ld) ... ", n);
  fflush(stderr);
  gettimeofday (&tv_begin, NULL);
  // warshall(n, a, c);
  parallel_warshall(n, a, c, num_procs, rank);
  int (*temp_c)[n] = malloc(sizeof(int[n][n]));
  memcpy(temp_c, c, sizeof(int[n][n]));
  MPI_Allreduce(temp_c, c, n * n, MPI_INT, MPI_LOR, MPI_COMM_WORLD);
  free(temp_c);
  gettimeofday (&tv_end, NULL);

  if(rank == 0){
    fprintf(stderr, " done.\n");
    print_matrix (NULL, n, c, 0, n, 0, n, 0, false);
  }


#ifdef CCOMP
  size_t ncomps;
  comp_t * ccomp = make_ccomp_digraph (n, c, &ncomps);

  free(a);
  free(c);
  if(rank == 0)
    fprintf (stderr, "* %ld connected components after make_ccomp_digraph.\n", ncomps);

  // write the graph transitivly closed in dot format
  char *output_file = malloc(strlen(OUTPUT_TYPE) + strlen(OUTPUT_EXT) + strlen(filename) + 1); 
  sprintf(output_file, "%s%s%s", filename, OUTPUT_TYPE, OUTPUT_EXT);
  FILE *fgraph_clos = fgraph_clos = fopen (output_file, "w");
  if (fgraph_clos == NULL) {
    fprintf (stderr, "Cannot open file %s for writing.\n", output_file);
    exit (EXIT_FAILURE);
  }
  print_dot (fgraph_clos, ccomp, ncomps);
  fclose (fgraph_clos);
#endif

  //---------------------------------------------------------------------------
  //Execution times
  if(rank == 0)
    fprintf (stderr, "Init : %lfs, Compute : %lfs\n", DIFFTEMPS (tv_init, tv_begin), DIFFTEMPS (tv_begin, tv_end));

  //MPI finalize
  MPI_Finalize();



  return 0;
}
