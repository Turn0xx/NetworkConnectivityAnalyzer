#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/errno.h>
#include "util.h"


/**
 * count number of 1 values on a line
 **/
size_t count_1 (int *line, size_t len)
{
      size_t total = 0;
      for (size_t i = 0; i < len; i++)
	    if (line[i] == 1)
		  total++;
      return total;
}

size_t find_max(char * filename, char *sep) {
    FILE * fin = fopen(filename, "r"); 
    if (fin == NULL) {
        fprintf(stderr, "Error : can't open file.\n");
        exit(EXIT_FAILURE);
    }
    size_t value1=-1, value2=-1;
    size_t max=0;  // all values are positive
    size_t line=0;
    char   format_in[16];
    sprintf(format_in, "%s%s%s", "\%ld", sep, "\%ld");
    int nb_read = fscanf(fin, format_in, &value1, &value2);
                   //
    //while ((fscanf(fin, "%ld%*c", &value) == 1)) {
    while ( nb_read == 2) {    // expect 2 values in each line
        if (value1 > max) max=value1;
        if (value2 > max) max=value2;
	  nb_read = fscanf(fin, format_in, &value1, &value2);
        line++;
    }
    if (nb_read == 0)
        printf("* warning: stop reading line %ld. Is the line ok?\n",line);
    fclose(fin);  // reset pointer to start
    return max;
}

/**
 *  print_dot
 *  print a compt_t list of elements to dot format.
 *  @param out : a FILE pointer to an open file
 *  @param ccomp : a list of compt_t elements to print
 *  @param num_comp : the number of elements
 **/
void print_dot (FILE * out, comp_t * ccomp, size_t num_comp)
{
	#define COMP_NAME "component_"
	fprintf(stderr,"* writing %ld components in dot format ... ", num_comp);
	fflush(stderr);
      fprintf (out, "graph {\n");
      for (size_t i = 0; i < num_comp; i++) {
	    fprintf(out,"\tsubgraph %s%ld {\n\t\t", COMP_NAME, i); 
	    for (size_t n=0; n < ccomp[i].len; n++) { 
		  fprintf (out, "%ld ", ccomp[i].nodes[n]);
	        if (n < ccomp[i].len-1) // not the last
		  	fprintf (out, " -- ");
		  else
		  	fprintf (out, ";\n");
	    }
	    fprintf(out,"\t}\n"); // end subgraph
      }
      fprintf (out, "}\n");
	fprintf(stderr, "done.\n");
}


/*
 * is_present
 * @brief given a node number `leader` and a storage of components `comp`
 * along with the number `num_comp` of components in the storage, tell if
 * `leader` appears in any component.
 */
bool is_present(size_t leader, comp_t * comp,  size_t num_comp) {
	  
	  if (num_comp == 0) // no component created yet
		return false; 
	  for (size_t i=0; i<num_comp; i++) {
		for (size_t j=0; j<comp[i].len ;j++) {
	  		if (comp[i].nodes[j] == leader) {
	  			//printf("-> found %ld in component #%ld\n", leader, i);
		   		return true;
			}
		}
	  }
	  //printf("-> %ld not found in components #0 .. #%ld\n", leader, num_comp-1);
	  return false; 
}

/*
 * converts a line with 1/0 to a list with only the connected node numbers
 * @param line  : a line of the adjacency matrix 
 * @param nodes : a list of values i where line[i] is non-null
 **/
void fill_nodes(int *line, size_t *nodes, int line_len) {
	size_t cnt = 0;
	for (size_t i=0; i<line_len; i++)
		if (line[i] != 0)
			nodes[cnt++] = i;
} 



/*
 * create a new component and add the initial node to it
 * increment the total number of components by 1
 */
void create_component(size_t node, size_t n , comp_t *comp, size_t * num_comp) {
	comp[*num_comp].nodes = malloc(n * sizeof(size_t));
	comp[*num_comp].len = 1;
	comp[*num_comp].nodes[0] = node;
	//printf("* created new component #%ld (nodes=%pd)\n", *num_comp, comp[*num_comp].nodes);
	(*num_comp)++;
}

/*
 * remove component
 * Given the list of components `comp` of length `num_comp` and an index `cn` 
 * in this list, s.t 0 <= cn < num_comp, make comp[cn] <- comp[cn+1], 
 * comp[cn+1] <- comp[cn+2], ...., comp[num_comp-2] <- comp[num_comp-1]
 *
 * @param cn : the component's number to remove
 * @param num_comp : length of the comp list, 
 */
int remove_component(size_t cn, comp_t * comp, size_t * num_comp) {

	if (cn > *num_comp) {
	  fprintf(stderr, "Internal error: component #%ld to remove greater than\
				max (%ld)\n", cn, *num_comp); 
		return -1;
	}
	// shift subsequent components overwritting component cn 
	for (size_t i=cn; i<(*num_comp)-1; i++) {
		//printf("* \tfree comp[%ld].nodes (%p) before <-  comp[%ld].nodes (%p)\n", i, comp[i].nodes, i+1, comp[i+1].nodes); 
		//free(comp[i].nodes);
	      comp[i].nodes = comp[i+1].nodes; 	
		comp[i].len = comp[i+1].len;
	}
	// free last
	//free(comp[*num_comp-1].nodes);
	comp[*num_comp-1].len = 0;
	(*num_comp)--;
	return 0; // no error
}


/*
 * mrge_comp
 * assert cc_len >= 1
 */
void merge_component(size_t candidate_comp[MAX_CC], size_t cc_len, comp_t * comp, size_t * num_comp) {

	  size_t dest = candidate_comp[0]; // merge into this one
	  // iterate over remaining comp 
	  for (size_t i=1; i<cc_len; i++) {
		    // iterate over nodes of i'th comp to add 
		    //printf("* merge component #%ld (len=%ld) with dst #%ld\n", candidate_comp[i], comp[candidate_comp[i]].len, dest);
		    for (size_t j=0; j<comp[candidate_comp[i]].len; j++) {
				// add at end of dst comp nodes' list (there is enough room malloc'd)
				comp[dest].nodes[comp[dest].len] = comp[candidate_comp[i]].nodes[j];
				comp[dest].len++;
		    }
	  }
	  // now components candidate_comp[1 ... cc_len-1] should be removed
	  for (size_t i=1; i<cc_len; i++) {
		    if (remove_component(candidate_comp[i], comp, num_comp)==0) {
				//printf("* component #%ld removed\n", candidate_comp[i]);
		    }	
	  }

}

/*
 * Add a node to one of the component of `comp` if node has a link with any node 
 * in that component.
 * If no link can be found, create a new component and put the node in it.
 *
 * return the component number to which it has been added
 * and update the number of components (+1 if one was created)
 */
void add_in_appropriate_comp(size_t node, size_t n, int m[n][n], comp_t * comp, size_t * num_comp) {
	size_t candidate_comp[MAX_CC];  // list of components which node could be connected to
	size_t cc_len = 0;              // retain
	 
	//printf("* try add %ld in components\n", node);
	for (size_t c=0; c < *num_comp; c++) {
		bool already_candidate = false;
		// iterate over nodes in component c
		for (size_t i=0; i<comp[c].len; i++) {
			size_t cnode = comp[c].nodes[i];
			//printf("* component #%ld (len=%ld): test if %ld->%ld =%d or %ld->%ld =%d\n", c, comp[c].len, node,cnode,m[node][cnode],cnode,node,m[cnode][node] );
			// check if it connected
			if ((m[node][cnode] || m[cnode][node]) && !already_candidate) {
				// node is connected to i (for the first time), so add component c in the list of candidates.
				// printf("* component #%ld added to candidates\n", c);
				candidate_comp[cc_len] = c;
				cc_len++;
				already_candidate = true; // don't add more than once this component to candidates
			}
		}
		//if (!already_candidate)
			  //printf("* node %ld not linked to nodes of component #%ld.\n", node, c);

	}
	// Any link found?
	//printf("* node has links in %ld components.\n", cc_len);
	if (cc_len == 0) {
		// node not present in an existing component.
		// create one and put node in it.
		//printf("* node %ld linked to none of the %ld component.\n", node, *num_comp);
		create_component(node, n, comp, num_comp);
	}
	if (cc_len >= 1) {
		// linked to a node of a single component: add node to this component
		size_t c =  candidate_comp[0]; 
		comp[c].nodes[comp[c].len] = node;
		comp[c].len++;
		// printf("* node %ld added into component #%ld. \n", node, c);
	}
	if (cc_len > 1) {
		// several connections in different components :
		// merge these components and add node to the merged component.
		// printf("* merge need for %ld components.\n", cc_len);
		merge_component(candidate_comp, cc_len, comp, num_comp); 
	}

}

/*  make_ccomp_digraph computes the sets of connected components from
 *  the adjency matrix transitively closed.
 *  @param m : the adjecy matrix
 *  @param num_comps : an out parameter, returning the number of connected 
 *              components found
 *  @return a structure with the different connected components found
 */
comp_t * make_ccomp_digraph (size_t n, int m[n][n], size_t * ncomps) {
	comp_t *ccomp = malloc(sizeof(comp_t) * MAX_CC); // later: use realloc
	size_t num_comp=0;
      // iterate over each node i (row number in matrix)
      for (size_t i = 0; i < n; i++) {
		add_in_appropriate_comp(i, n, m, ccomp, &num_comp);
		// printf("* node %ld added to component #%ld\n", i, c);
	}
	*ncomps = num_comp;
      return ccomp;
}

/*
 * display a matrix.
 * @param label : a string to name the display
 * @param m : a square matrix 
 * @param  len : length
 * @param rank : a process number (put 0 in sequential)
 * @param initializer_style : if true, prints in form more easy to copy/paste as a C array
 *        initializer
 */
void print_matrix (char * label, size_t const n, int matrix[n][n], 
		                     size_t lbi, size_t ubi, 
					   size_t lbj, size_t ubj, 
					   int rank, bool initializer_style) 
{
	sleep(rank);
	char sep;
	size_t cnt = 0;
	sep = initializer_style ? ',' : ' ';
	if (label)
		printf("[%s@%d]", label, rank);
	if  (initializer_style) 
		printf("{\n");
	else {
      	if (label) {
			for (size_t j = lbj; j < ubj; j++)
	    			printf ("--");
      		printf ("\n");
		}
	}
      for (size_t i = lbi; i < ubi; i++) {
	  	if (initializer_style)
			printf("{");
	    	for (size_t j = lbj; j < ubj; j++) {
		  	printf ("%1d", matrix[i][j]);
	  		if (j!= ubj-1)
				printf("%c", sep);
		}
	    	if (initializer_style)
			printf("},");
	    	printf ("\n");
		cnt++;
      }
	fprintf(stderr,"* print_matrix(): printed %ld lines.\n", cnt);
}


size_t matrix_lines_from_file(char * filename) {
    FILE *fp;
    size_t count = 0;
    int c;

    fp = fopen(filename, "r"); 
    if (fp == NULL) {
        fprintf(stderr, "Error : can't open file.\n");
        exit(EXIT_FAILURE);
    }
    for (c = getc(fp); c != EOF; c = getc(fp))
        if (c == '\n') // count number of newline characters
            count++;
    fclose(fp);
    return count;
}


void matrix_from_adj_file(char * filename, size_t n, int matrix[n][n]) {
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(filename, "r"); 
    if (fp == NULL) {
        fprintf(stderr, "Error : can't open file.\n");
        exit(EXIT_FAILURE);
    }

    for (size_t i=0; i < n; i++) {
        read = getline(&line, &len, fp);
	  if (read==-1) {
        		fprintf(stderr, "Error reading file.\n");
        		exit(EXIT_FAILURE);
	  }
        char *tok = strtok(line, " ");
        for (size_t j = 0; j < n; j++) {
            matrix[i][j] = atoi(tok);
            tok = strtok(NULL, " ");
        }
    }
    fclose(fp);
}


void matrix_from_pairs_file(char * filename, size_t n, int matrix[n][n], char *sep) {

    FILE * fin = fopen(filename, "r"); 
    if (fin == NULL) {
        fprintf(stderr, "Error : can't open file.\n");
        exit(EXIT_FAILURE);
    }
    size_t value1=-1, value2=-1;
    size_t line=0;
    char   format_in[16];
    sprintf(format_in, "%s%s%s", "\%ld", sep, "\%ld");
    int nb_read = fscanf(fin, format_in, &value1, &value2);
    matrix[value1][value2] = 1;

    while ( nb_read == 2) {    // expect 2 values in each line
    	nb_read = fscanf(fin, format_in, &value1, &value2);
	matrix[value1][value2] = 1;
      line++;
    }
    if (nb_read == 0)
        fprintf(stderr, "* warning: stop reading line %ld. Is the line ok?\n",line);
    fprintf(stderr, "* matrix_from_pairs_file(): read %ld pairs.\n", line);
}
