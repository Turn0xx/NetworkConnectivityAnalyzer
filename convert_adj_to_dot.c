#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "util.h"

#define EXT ".dot"
#define MEGA (size_t)(1 << 20)

/* 
 * Fill an adjacency matrix for an undirected graph from a file.
 * The file format is a set of pairs:  node1,node2  where node1 and node2 are 
 * integer numbers starting from 0.
 * There is one pair per line, and expresses an undirected edge node1 -- node2.
 * The adjacency matrix m is stored as a linear block of memory of size max * max 
 * booleans. max is the greatest node number found in the file.
 * For each node1 -- node2 relation, m is set like this m[node1][node2] = true and
 * m[node2][node1] = true.
 **/
void fill(bool ** m, size_t max, FILE *fin) {
    size_t value1=-1, value2=-1;
    size_t nb_read;
    while ((nb_read = fscanf(fin, "%ld,%ld", &value1, &value2)) == 2) {
        // linearize: size_t indx1 = value1 + value2 * max;
        // linearize: size_t indx2 = value2 + value1 * max;
        m[value1][value2] = true;
        m[value2][value1] = true;
    }
}


bool ** alloc_mem(size_t count) {
    bool ** m = malloc((count+1)*sizeof(bool *));
    for (size_t i=0; i<count+1; i++) {
       m[i] = malloc((count+1)*sizeof(bool));
       assert(m[i]);
    }
    if (m)
    	 printf("* malloc'd %ld MBytes\n", (count+1)*(count+1)/MEGA);
    return m;
}

/**
 *  free mem
 **/
void free_mem(bool ** m, size_t count) {
    for (size_t i=0; i< count+1; i++)
       free(m[i]);
    free(m);
}

/**
 *  print_mat2dot
 **/
void print_mat2dot (FILE * out, size_t n, int  m[n][n])
{
      bool isolated_node;
      fprintf (out, "digraph {\n");
      for (size_t i = 0; i < n; i++) {
        isolated_node = true;
	    for (size_t j=0; j < n; j++) { 
		  if (m[i][j]) {
			fprintf (out, "%ld -> %ld;\n", i, j);
            isolated_node = false;
           } 
	    }
        if (isolated_node)
			fprintf (out, "%ld;\n", i);
      }
      fprintf (out, "}\n");
}


/*
 * main
 */
int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s filename", argv[0]); 
        return 1;
    }
    FILE *fin = fopen(argv[1],"r");
    if (!fin) {
        fprintf(stderr,"* Could not open %s fo reading. Exiting.\n", argv[1]);
	    exit(1);
    }	

	// first get the adjacency matrix size
	size_t n = matrix_lines_from_file(argv[1]);
	fprintf(stderr, "* %s has %ld lines.\n", argv[1], n);

	// malloc a VLA as a pointer to an array of n integers
    int (*matrix)[n] = malloc(sizeof(int[n][n]));
	// read data from a file containing an adjacency matrix
	matrix_from_adj_file(argv[1], n, matrix);

    size_t name_len = strlen(argv[1])+strlen(EXT)+1;
    char *filename_out = malloc(name_len);
    snprintf(filename_out, name_len, "%s%s", argv[1], EXT);
    FILE *fout = fopen(filename_out,"w");
    printf("* about to write to %s in dot format ... ", filename_out);
    fflush(stdout);
    
    print_mat2dot(fout, n , matrix);
    fclose(fout);
    printf("done.\n");
    free(matrix);
    return 0;
}
