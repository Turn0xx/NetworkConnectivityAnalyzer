#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#define MAX_CC 500  // maximum number of connected components


/* one component a list of nodes + list length */
typedef struct {
	  size_t *nodes;
	  size_t len;
} comp_t;


/**
 * count number of 1 values on a line
 **/
size_t count_1 (int *line, size_t len);

/*
 * return the max value in file
*/
size_t find_max(char * filename, char * sep);


/**
 *  print_dot
 *  print a compt_t list of elements to dot format.
 *  @param out : a FILE pointer to an open file
 *  @param ccomp : a list of compt_t elements to print
 *  @param num_comp : the number of elements
 **/
void print_dot (FILE * out, comp_t * ccomp, size_t num_comp);

/*
 * is_leader_present
 * @brief given a node number `leader` and a storage of components `comp`
 * along with the number `num_comp` of components in the storage, tell if
 * `leader` appears in any component.
 */
bool is_leader_present(size_t leader, comp_t * comp,  size_t num_comp);

 /*
 * converts a line with 1/0 to a list with only the connected node numbers
 * @param line  : a line of the adjacency matrix 
 * @param nodes : a list of values i where line[i] is non-null
 **/
void fill_nodes(int *line, size_t *nodes, int line_len);

/*
 * return the position of the first connected node
 * return -1 if line has only 0s
 **/
size_t first_connected(int * line, size_t len); 

/**
 *  make_ccomp
 *  Converts an adjency matrix representing the transitive closure to a data
 *  structure storing connected components.
 *  
 *  Algorithm for undirected graph:
 *  The transitive closure matrix has identical lines for all nodes that belongs
 *  to a same component.
 *  For example, line 0 in the following describes adjencies 0-0, 0-2, 0-4, 0-6, 0-7
 *  hence nodes 0,2,4,6,7 are in the same component. It means lines 2, 4, 6, and 7 
 *  will be the same component as line 0.
 *
 *  0 1 2 3 4 5 6 7 8 9   node id
 *  --------------------+
 *  1 0 1 0 1 0 1 1 0 0 | 0
 *  0 1 0 1 0 1 0 0 0 0 | 1
 *  1 0 1 0 1 0 1 1 0 0 | 2
 *  0 1 0 1 0 1 0 0 0 0 | 3
 *  ...... 
 *
 *  We want to convert only one of this line for each component.
 *  In the example we can simplify by keeping only line 0.
 *  In the case of undirected graph, it suffices to find in each line, if the row-wise
 *  first connected node (so the one with lowest number) is present as the first
 *  element of an already stored component. If yes, just skip this line. Otherwise,
 *  store all nodes numbers that are connected into a new component.
 *
 *  @param m : an adjacency matrix with all transitive connections between nodes 
 *  @param lines : the number of connected components, whcih is the number
 *                 of unique lines in m
 *  @return : a pointer to a list of comp_t structures, one for each component
 **/
comp_t * make_ccomp_digraph (size_t n, int m[n][n], size_t * ncomps);

/*
 * display a matrix.
 * @param label  : a string to name the display
 * @param n      : number of lines of the matrix
 * @param matrix : a square matrix 
 * @param lbi    : lower bound on lines  
 * @param ubi    : upper bound on lines  
 * @param lbj    : lower bound on columns  
 * @param ubj    : upper bound on columns  
 * @param rank   : a process number (put 0 in sequential)
 * @param initializer_style : if true, put braces around values
 */
void print_matrix (char * label, size_t n, int matrix[n][n], 
		                     size_t lbi, size_t ubi, 
					   size_t lbj, size_t ubj, 
					   int rank, bool initializer_style);

/*
 * count the number of lines in the files
 */
size_t matrix_lines_from_file(char * filename);


/*
 * Read an adjacency matrix. 
 * load a matrix of integers from a file to a 2D buffer.
 * The buffer is `matrix` and must allocated for n x n elements prior
 * to the function call.
 * `matrix` is updated on return.
 *
 * BEWARE: the function is fragile, segfault will occur if the file read
 * has not *exactly* the number of lines `n` advertized.  
 */
void matrix_from_adj_file(char * filename, size_t n, int matrix[n][n]);

void matrix_from_pairs_file(char * filename, size_t n, int matrix[n][n], char *sep);

#endif
