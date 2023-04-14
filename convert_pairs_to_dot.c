#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "util.h"   // for find_max()

#define SEP "\t"
#define EXT ".dot"
#define MEGA (size_t)(1 << 20)


/* 
 * Fill an adjacency matrix for an undirected graph from a file.
 * The file format is a set of pairs:  node1<sep>node2  where node1 and node2 are 
 * integer numbers starting from 0.
 * <sep> is a separator string to be set in #define ahead of this file
 * There is one pair per line, and expresses an undirected edge node1 -- node2.
 * The adjacency matrix m is stored as a linear block of memory of size max * max 
 * booleans. max is the greatest node number found in the file.
 * For each node1 -- node2 relation, m is set like this m[node1][node2] = true and
 * m[node2][node1] = true.
 **/
void fill(bool ** m, size_t max, FILE *fin, char *sep) {


    size_t value1=-1, value2=-1;
    size_t line=0;
    char   format_in[16];
    sprintf(format_in, "%s%s%s", "\%ld", sep, "\%ld");
    int nb_read = fscanf(fin, format_in, &value1, &value2);
    m[value1][value2] = true;
    m[value2][value1] = true;

    while ( nb_read == 2) {    // expect 2 values in each line
    	nb_read = fscanf(fin, format_in, &value1, &value2);
      m[value1][value2] = true;
      m[value2][value1] = true;
      line++;
    }
    if (nb_read == 0)
        fprintf(stderr, "* warning: stop reading line %ld. Is the line ok?\n",line);
    fprintf(stderr, "* matrix_from_pairs_file(): read %ld pairs.\n", line);
}


bool ** alloc_mem(size_t count) {
    bool ** m = malloc((count+1)*sizeof(bool *));
    for (size_t i=0; i<count+1; i++) {
       m[i] = malloc((count+1)*sizeof(bool));
       assert(m[i]);
    }
    if (m)
    	 printf("* malloc'd %.2lf MBytes\n", ((double)((count+1)*(count+1))/MEGA));
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
void print_mat2dot (FILE * out, bool ** m, size_t count)
{
	fprintf(stderr, "count=%zu\n", count);
      fprintf (out, "graph {\n");
      for (size_t i = 0; i < count+1; i++) {
	    for (size_t j=0; j < count+1; j++) { 
		  if (m[i][j])
			fprintf (out, "%ld -- %ld;\n", i, j);
	    }
      }
      fprintf (out, "}\n");
}



int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Usage: %s filename", argv[0]); 
        return 1;
    }
    FILE *fin = fopen(argv[1], "r");
    if (!fin) {
	fprintf(stderr,"* Could not open %s fo reading. Exiting.\n", argv[0]);
	exit(1);
    }	
    size_t max = find_max(argv[1], SEP);
    printf("* max id node found=%ld\n", max);
    bool ** m = alloc_mem(max);
    fill(m, max, fin, SEP);
    fclose(fin);

    size_t name_len = strlen(argv[0])+strlen(EXT)+1;
    char *filename_out = malloc(name_len);
    snprintf(filename_out, name_len, "%s%s", argv[0], EXT);
    FILE *fout = fopen(filename_out,"w");
    printf("* about to write to %s in dot format ... ", filename_out);
    fflush(stdout);
    
    print_mat2dot(fout, m, max);
    fclose(fout);
    printf("done.\n");
    free_mem(m, max);
    return 0;
}
