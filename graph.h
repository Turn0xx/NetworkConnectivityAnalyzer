#ifndef __GRAPH_H__ 
#define __GRAPH_H__


// Probability to set an edge between two nodes in case of random init.
#define PROBA_CONN 0.01


#define INPUT_TYPE_ADJ  "adj"
#define INPUT_TYPE_PAIRS "pairs"

#define OUTPUT_TYPE 	"-tclos"
#define OUTPUT_EXT 	".dot"


void usage(char *argv[]) {
	printf("usage: %s -i input_file -t type=%s|%s\n", argv[0],INPUT_TYPE_ADJ, INPUT_TYPE_PAIRS); 
	exit(EXIT_FAILURE);
}


void parse_options(int argc, char *argv[], char ** filename, char ** input_type) {
	int opt;
	// put ':' in the starting of the
	// string so that program can
	//distinguish between '?' and ':'
	while((opt = getopt(argc, argv, ":i:t:")) != -1)
	{
		switch(opt)
		{
			case 'i':
				*filename = strdup(optarg);
				break;
			case 't':
				*input_type = strdup(optarg);
				break;
			case 'h':
				usage(argv);
				break;
			case ':':
				printf("The option needs a value\n");
				usage(argv);
				break;
			case '?':
				printf("Unknown option: %c\n", optopt);
				usage(argv);
				break;
		}
	}
	// optind is for the extra arguments
	// which are not parsed
	bool ok = true;
	for(; optind < argc; optind++){	
		printf("Extra arguments: %s\n", argv[optind]);
		ok = false;
	}
	if (!ok)
		usage(argv);

}

#endif
