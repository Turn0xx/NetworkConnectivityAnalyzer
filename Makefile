
CC=gcc
MPICC=OMPI_CC=$(CC) mpicc    # make OpenMPI's mpicc use the compiler you wish
OMP=-fopenmp
MD5=md5sum

# possibles flags 
# -DPROGRESS : to display an indicator of computation progress
# -DDCOMP : to compute connected components after transitive closure
# -DDEBUG : display various debugging messages

CFLAGS=-O3 -Wall -g -DPROGRESS #-DCCOMP # -DDEBUG
DATADIR=share
RESDIR_SEQ=results-seq
RESDIR_PAR=results-par
DATASET_1000=1000x1000.adj
DATASET_3000=3000x3000.adj
DATASET_10000=10000x10000.adj
TCLOS_EXT=tclos.adj
MPI_EXT=mpirun-n


BINARIES= graph graph_par convert_adj_to_dot convert_pairs_to_dot



all: $(BINARIES)


graph : graph.o util.h util.o graph.h
	$(CC) $(CFLAGS) graph.o util.o -o $@

graph.o : graph.c util.h graph.h
	$(CC) $(CFLAGS) -c $<

util.o : util.c util.h graph.h
	$(CC) $(CFLAGS) -c $<

graph_omp : graph_omp.o util.h util.o graph.h
	$(CC) $(CFLAGS) $(OMP) graph_omp.o util.o -o $@

graph_omp.o : graph_omp.c util.h graph.h
	$(CC) $(CFLAGS) $(OMP) -c $<

graph_par : graph_par.o util.h util.o graph.h
	$(MPICC) $(CFLAGS) $(OMP) graph_par.o util.o -o $@

graph_par.o : graph_par.c util.h graph.h
	$(MPICC) $(CFLAGS) $(OMP) -c $<


convert_adj_to_dot : convert_adj_to_dot.o util.o util.h
	$(CC) $(CFLAGS) convert_adj_to_dot.o util.o -o $@

convert_pairs_to_dot : convert_pairs_to_dot.o util.o util.h
	$(CC) $(CFLAGS) convert_pairs_to_dot.o util.o -o $@


# ----------------- generate data ---------------------
$(DATADIR)/$(DATASET_1000) : 
	@./gen_adj.py 1000 4 > $(DATADIR)/$(DATASET_1000) 

$(DATADIR)/$(DATASET_3000) :
	@echo "Generating $(DATADIR)/$(DATASET_3000) ..." 
	@./gen_adj.py 3000 4 > $(DATADIR)/$(DATASET_3000) 

$(DATADIR)/$(DATASET_10000): 
	@echo "Generating $(DATADIR)/$(DATASET_10000) ..." 
	@./gen_adj.py 10000 4 > $(DATADIR)/$(DATASET_10000) 

# ----------------- runs ------------------------------
run-seq-1000: graph $(DATADIR)/$(DATASET_1000) 
	@mkdir -p $(RESDIR_SEQ)
	@echo "===== $@ ======"
	./graph -i $(DATADIR)/$(DATASET_1000) -t adj > $(RESDIR_SEQ)/$(DATASET_1000)-$(TCLOS_EXT)

run-seq-3000: graph $(DATADIR)/$(DATASET_3000) 
	@mkdir -p $(RESDIR_SEQ)
	@echo "===== $@ ======"
	./graph -i $(DATADIR)/$(DATASET_3000) -t adj > $(RESDIR_SEQ)/$(DATASET_3000)-$(TCLOS_EXT)

run-seq-10000: graph $(DATADIR)/$(DATASET_10000)
	@echo "Warning : will run for 10 minutes ..." 
	@mkdir -p $(RESDIR_SEQ)
	@echo "===== $@ ======"
	./graph -i $(DATADIR)/$(DATASET_10000) -t adj > $(RESDIR_SEQ)/$(DATASET_10000)-$(TCLOS_EXT)

run-par-1000: graph_par $(DATADIR)/$(DATASET_1000) 
	@mkdir -p $(RESDIR_PAR)
	@echo "===== $@ OMP=$(OMP)======"
	for np in 4; do \
		echo ">>> -n $$np"; \
		mpirun -n $$np ./graph_par -i $(DATADIR)/$(DATASET_1000) -t adj > $(RESDIR_PAR)/$(DATASET_1000)-$(MPI_EXT)$$np-$(TCLOS_EXT); \
		echo ; \
	done

run-par-3000: graph_par $(DATADIR)/$(DATASET_1000) 
	@mkdir -p $(RESDIR_PAR)
	@echo "===== $@ OMP=$(OMP)======"
	for np in 4; do \
		echo ">>> -n $$np"; \
		mpirun -n $$np -x OMP_NUM_THREADS=4 ./graph_par -i $(DATADIR)/$(DATASET_3000) -t adj > $(RESDIR_PAR)/$(DATASET_3000)-$(MPI_EXT)$$np-$(TCLOS_EXT); \
		echo ; \
	done

run-par-10000: graph_par $(DATADIR)/$(DATASET_10000)
	@mkdir -p $(RESDIR_PAR)
	@echo "===== $@ OMP=$(OMP)======"
	for np in 4; do \
		echo ">>> -n $$np"; \
		mpirun -n $$np  ./graph_par -i $(DATADIR)/$(DATASET_10000) -t adj > $(RESDIR_PAR)/$(DATASET_10000)-$(MPI_EXT)$$np-$(TCLOS_EXT); \
		echo ; \
	done

# ----------------- tests ------------------------------
test-seq-1000: run-seq-1000

test-seq-3000: run-seq-3000
	@echo "MD5"
	@echo "$(shell $(MD5) $(RESDIR_SEQ)/$(DATASET_3000)-$(TCLOS_EXT) )"

test-seq-10000: run-seq-10000 

test-par-1000: run-par-1000
	@echo "$(shell cmp $(RESDIR_PAR)/$(DATASET_1000)-$(MPI_EXT)*-$(TCLOS_EXT) $(RESDIR_SEQ)/$(DATASET_1000)-$(TCLOS_EXT))"

test-par-3000: run-seq-3000 run-par-3000
	@echo "$(shell cmp $(RESDIR_PAR)/$(DATASET_3000)-$(MPI_EXT)*-$(TCLOS_EXT) $(RESDIR_SEQ)/$(DATASET_3000)-$(TCLOS_EXT))"
	@echo "MD5 $(RESDIR_PAR)/$(DATASET_3000)-$(MPI_EXT)*-$(TCLOS_EXT)"
	@echo "$(shell $(MD5) $(RESDIR_PAR)/$(DATASET_3000)-$(MPI_EXT)*-$(TCLOS_EXT) )"

test-par-10000: run-par-10000 
	@echo "$(shell cmp $(RESDIR_PAR)/$(DATASET_10000)-$(MPI_EXT)*-$(TCLOS_EXT) $(RESDIR_SEQ)/$(DATASET_10000)-$(TCLOS_EXT))"
	@echo "MD5"
	@echo "$(shell $(MD5) $(RESDIR_PAR)/$(DATASET_10000)-$(MPI_EXT)*-$(TCLOS_EXT) )"


# ----------------- tests ------------------------------
clean:
	rm -rf *.o $(BINARIES) *.dSYM *~
