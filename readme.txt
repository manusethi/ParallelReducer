Project 3

Task 1

compile as:

g++ -fopenmp project3_task1.cpp -o project3_task1

run as:
./project3_task1 facebook_combined.txt


Notes:
(1) The code itself finds the number of processors in the computer on which this code is run and then creates an equal number of threads. For example, on intel core i7 it creates 8 threads. This is done using the functions omp_get_max_threads() and omp_set_num_threads(nProcessors). 

(2) The value of d for teleport is kept to be the standard value of 0.85 as was in project 1 (which was done on hadoop).

(3) The page rank calculation stops when the difference between the l-1 norms of the last and next pagerank values divided by the total nodes is less than 1e-8.

(4) On intel core i7 it takes about 5 seconds and 28 iterations to converge.

(5) The code is tested on facebook_combined.txt graph. This graph is treated as undirected and so inlink and outlink graph will be exactly the same. This also means that there are no dead ends in the graph.

(6) The initial pagerank vector is chosen to be of unit norm where each entry is 1/number_of_nodes.

(7) For efficiency and to save memory the graph is stored in a sparse matrix format and matrix vector multiplication is done accordingly.

=================================================================================================================================
Task 2

compile as:

mpic++ project3_task2.cpp

run as:

mpirun -np 4 ./a.out <inputfilename>


Notes:

(1) The code is tested using 4 processors. It should work with a different number of processors also but I have not tested that.

(2) Only collective communication is used in this code for efficiency and we do not use point to point communication.

(3) The task is carried in two steps. In first step the key value pairs are read by the master and divided equally using MPI_Scatterv across all processes.

(4) Local reduction happens in each processors by storing the values in a hashtable or specifically STL map (using red black tree implementation of C++ STL map).

(5) After local reduction each key is mapped to a processor using cyclical fashion. This is done by finding "key % nprocs". For mapping we use MPI_Alltoallv() function.

(6) Now again the values are reduced locally using the STL map.

(7) After this second reduction we use MPI_Gatherv to send the elements back to the root process.

(8) The root/ master then writes the key and reducedsum pairs to the output file.
