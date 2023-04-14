#!/usr/bin/env python3

# S. Genaud, march 2023

# Generates an adjacency matrix for a graph of n nodes with a given number of
# connected components.

# The idea is to buid separate adj matrices for each connected component,
# for example #nodes=6, comp A. with 4 nodes, comp B. with 2 nodes.

#   A          B            final adj. mtrix
#
# 0 1 0 0      0 1          0 1 0 0 | 0 0
# 0 0 1 0      1 0    =>    0 0 1 0 | 0 0
# 0 1 0 0                   0 1 0 0 | 0 0
# 0 0 1 0                   0 0 1 0 | 0 0
#                           --------+----
#                           0 0 0 0 | 0 1
#                           0 0 0 0 | 1 0



import sys
import random

PROBA_CONN = 0.1    # probability to connect two nodes

if len(sys.argv) != 3:
    print(f"Usagge : {sys.argv[0]} <total number of nodes> <number of components>")
    sys.exit(1)

n = int(sys.argv[1])
ncomps = int(sys.argv[2])


'''
Two square matrices
'''
def merge_as_diagonal_blocks(a, b):
    a_0 = [0]*len(a)
    b_0 = [0]*len(b)
    c = []
    for i in range(len(a)):
        c += [a[i] + b_0]
    for i in range(len(b)):
        c += [a_0 + b[i]]
    return c


def init_matrix(size):
    m = []
    for i in range(size):
        line = []
        for j in range(size):
            if (random.random()< PROBA_CONN):
                line += [1]
            else:
                line += [0]
        m += [line]
    return m


def print_matrix(m):
    for i in range(len(m)):
        for j in range(len(m[0])):
            print(f"{m[i][j]} ", end='')
        print()



random.seed(0);

# init the components' size
size_comps = []  # pair (start_index,len)
blocks = []
sum=0
for i in range(ncomps-1):  # does not get inside if comps==0
    block_size = n//ncomps
    sum += block_size
    size_comps += [block_size]
    blocks.append(init_matrix(block_size))
# the rest
size_comps.append(n-sum);
blocks.append(init_matrix(n-sum))

# construct adjacency matrix with diagonal blocks
blocks.append(init_matrix(size_comps[0]))   # blocks[0]
for i in range(1, ncomps):
    m = merge_as_diagonal_blocks(blocks[i-1], blocks[i])
    blocks[i] = m
print_matrix(m)

