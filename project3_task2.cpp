//============================================================================
// Name        : project3b.cpp
// Author      : ms
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <mpi.h>
#include <vector>
#include <map>
#include <list>
#include <numeric>

using namespace std;

void check_err(int err){

	if (err == MPI_SUCCESS){
		cout << "no error" << endl ;
		return;
	} else {
		cout << "here!" << endl ;
		int len, eclass;
		char estring[MPI_MAX_ERROR_STRING];
		MPI_Error_class(err, &eclass);
		MPI_Error_string(err, estring, &len);
		printf("Error %d: %s\n", eclass, estring);
		fflush(stdout);
		// fail("MPI Error !\n");
	}
}

int computeLocalReduce(int* &localUniqueKeys, int* &localSums, int* keyArr, int* valArr, int n){
	/*for (int i = 0 ; i < n ; i++)
		if (keyArr[i] == 9627) cout << "val corresponding to key 9627 is " << valArr[i] << endl ;*/
	map<int, list<int> > myhashtable ;
	map<int, list<int> >::iterator it ;
	for (int i = 0 ; i < n ; i++){
		if (keyArr[i] != -1)
			if (myhashtable.count(keyArr[i]) > 0)
				myhashtable.at(keyArr[i]).push_back(valArr[i]) ;
			else{
				list<int> temp ;
				temp.push_back(valArr[i]) ;
				myhashtable[keyArr[i]] = temp ;
			}
	}


	/*if (myhashtable.count(9627) > 0)
		for (list<int>::iterator llit = myhashtable[9627].begin() ; llit != myhashtable[9627].end() ; ++llit)
			cout << "9627 here you go " << *llit << endl ;
	 */

	int numUniqueKeys = myhashtable.size() ;
	localSums = new int [numUniqueKeys] ;
	localUniqueKeys = new int [numUniqueKeys] ;
	int i = 0 ;
	list<int> temp ;
	for (it = myhashtable.begin() ; it != myhashtable.end() ; ++it){
		temp = it->second ;
		localSums[i] = 0 ;
		/*for (list<int>::iterator lit = temp.begin() ; lit != temp.end() ; ++lit)
			if (it->first == 9627) cout << "lit val is " << it->first << "\t" << *lit << endl ;*/
		// localSums[i] += (*lit) ;
		localSums[i] = accumulate(it->second.begin(), it->second.end(), 0) ;
		if (it->second.size()==0)
			cout << "list size = " << it->second.size() << endl ;
		localUniqueKeys[i] = it->first ;
		i += 1 ;
	}

	return numUniqueKeys ;

}

int main(int argc, char *argv[]) {
	// cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	int nprocs, rank, numPairs, recvcount, localKeyCount ;
	int nMinPairsPerProcess ;
	int nExtraPairs ;
	int numElementsPerProcess ;
	int*  recvBufferKey, *recvBufferVal, *sendcounts, *displs, *localReduceSumsArr, *localKeys ;
	int* keyArr ;
	int* valArr ;
	int* getBack ;

	MPI_Init(NULL, NULL) ;
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs) ;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank) ;
	cout << "num of processes is " << nprocs << endl ;
	// only  Master process reads from file
	if (rank == 0){
		ifstream infile ;
		// infile.open("kvpairs.txt") ;
		infile.open(argv[1]) ;
		// int key, val ;
		string line ;
		numPairs = 0 ;

		if(infile.is_open()){
			while (getline(infile, line))
				++numPairs ;
			numPairs -= 1 ; // removing from count the first pair
			cout << "numPairs " << numPairs << endl ;
		} // end if
		else
			cout << "file could not be opened" << endl ;

		infile.close() ;


		keyArr = new int [numPairs] ;
		valArr = new int [numPairs] ;
		getBack = new int [numPairs] ;




		infile.open(argv[1]) ;

		char commaChar ;
		int key, val, i = 0 ;
		getline(infile, line) ; // skip the first line
		while (getline(infile, line)){
			sscanf(line.c_str(), "%d%c%d%c", &key, &commaChar, &val, &commaChar) ;

			keyArr[i] = key ;
			valArr[i] = val ;

			i += 1 ;
			/*if (i == 1 || i == 100000 || i == 200)
				cout << "key = " << key << "\t" << "val = " << val << "\t" << "rank = " << rank <<  endl ;*/
			/*if (key == 9627)
							cout << "key = " << key << "\t" << "val = " << val << "\t" << "rank = " << rank <<  endl ;*/
		} // end while for reading into the master process arrays


		// set the send counts
		nMinPairsPerProcess = numPairs / nprocs ;
		nExtraPairs = numPairs % nprocs ;

		sendcounts = new int [nprocs] ;

		displs = new int [nprocs] ;

		int k = 0 ;
		for (int i = 0 ; i < nprocs ; i++) {
			if (i < nExtraPairs) sendcounts[i] = nMinPairsPerProcess + 1 ;
			else sendcounts[i] = nMinPairsPerProcess ;
			//recvcounts[i] = sendcounts[i] ;
			displs[i] = k ;
			k += sendcounts[i] ;
		}

		recvcount = nMinPairsPerProcess + 1 ;

	} // end if rank == 0 for master process read
	MPI_Barrier(MPI_COMM_WORLD) ;

	MPI_Bcast(&numPairs, 1, MPI_INT, 0, MPI_COMM_WORLD) ;
	MPI_Bcast(&recvcount, 1, MPI_INT, 0, MPI_COMM_WORLD) ;
	MPI_Barrier(MPI_COMM_WORLD) ;

	recvBufferKey = new int [recvcount] ;
	recvBufferVal = new int [recvcount] ;

	MPI_Scatter(sendcounts, 1, MPI_INT, &numElementsPerProcess,
			1, MPI_INT, 0, MPI_COMM_WORLD) ;
	MPI_Scatterv(keyArr, sendcounts, displs, MPI_INT,
			recvBufferKey, recvcount, MPI_INT, 0, MPI_COMM_WORLD) ;
	MPI_Scatterv(valArr, sendcounts, displs, MPI_INT,
			recvBufferVal, recvcount, MPI_INT, 0, MPI_COMM_WORLD) ;
	MPI_Barrier(MPI_COMM_WORLD) ;

	/*for (int i = 0 ; i < numElementsPerProcess ; i++)
		if (recvBufferKey[i] == 9627)
			cout << "Hi val is " << recvBufferVal[i] << "  and rank is  " << rank << endl ;*/

	localKeyCount  = computeLocalReduce(localKeys, localReduceSumsArr, recvBufferKey,
			recvBufferVal, numElementsPerProcess) ;

	cout << "localKeyCount = " << localKeyCount << endl ;
	for (int i = 0 ; i < localKeyCount ; i++)
		if (localKeys[i] == 9635)
			cout << "localKey " << localKeys[i] << "\t" << localReduceSumsArr[i] << "\t" << rank <<  endl ;




	cout << "numPairs "<< numPairs << "  rank = " << rank << endl ;

	cout << "Per process elems " << numElementsPerProcess << endl ;

	// ================= Second step reduction ============================== all to all v?? =======

	int* step1Counts = new int [nprocs] ;

	MPI_Gather(&localKeyCount, 1, MPI_INT, step1Counts, 1, MPI_INT, 0, MPI_COMM_WORLD) ;
	MPI_Barrier(MPI_COMM_WORLD) ;


	// find the max
	int maxlen = 0 ;
	if (rank == 0)
	for (int k = 0 ; k < nprocs ; k++)
		if (step1Counts[k] > maxlen)
			maxlen = step1Counts[k] ;

	MPI_Barrier(MPI_COMM_WORLD) ;

	MPI_Bcast(&maxlen, 1, MPI_INT, 0, MPI_COMM_WORLD) ;


	// int maxlen = 10000 ;
	int* sBufKey = new int [maxlen] ;
	int* rBufKey = new int [nprocs*maxlen] ;

	int* sBufVal = new int [maxlen] ;
	int* rBufVal = new int [nprocs*maxlen] ;

	for (int i = 0 ; i < maxlen ; i++){
		sBufKey[i] = -1 ;

		sBufVal[i] = -1 ;

	}

	for (int i = 0 ; i < nprocs*maxlen ; i++){

			rBufKey[i] = -1 ;

			rBufVal[i] = -1 ;
		}

	int* scount = new int [nprocs] ;
	int* sdispls = new int [nprocs] ;
	int* rcount = new int [nprocs] ;
	int* rdispls = new int [nprocs] ;


	int cnt = 0 ;
	for (int k = 0 ; k < nprocs ; k++){
		scount[k] = 0 ;
		for (int i = 0 ; i < localKeyCount ; i++)
			if (localKeys[i] % nprocs == k){
				sBufKey[cnt] = localKeys[i] ;
				sBufVal[cnt] = localReduceSumsArr[i] ;
				cnt += 1 ;
				scount[k] += 1 ;
			}
		sdispls[k] = cnt - scount[k] ;
		rcount[k] = maxlen ;
		rdispls[k] = k*maxlen ;
	}

	MPI_Barrier(MPI_COMM_WORLD) ;

		MPI_Alltoallv(sBufKey, scount, sdispls, MPI_INT,
				rBufKey, rcount, rdispls, MPI_INT, MPI_COMM_WORLD) ;

		MPI_Alltoallv(sBufVal, scount, sdispls, MPI_INT,
				rBufVal, rcount, rdispls, MPI_INT, MPI_COMM_WORLD) ;

		MPI_Barrier(MPI_COMM_WORLD) ;



	int localKeyCount1 ;
	int* localKeys1, *localReduceSumsArr1 ;


	localKeyCount1  = computeLocalReduce(localKeys1, localReduceSumsArr1, rBufKey,
				rBufVal, nprocs*maxlen) ;

	/*for (int i = 0 ; i < localKeyCount1 ; i++)
		if (localKeys1[i] == 9627)
			cout << "localKey1 " << localKeys1[i] << "\t" << localReduceSumsArr1[i] << "\t" << rank <<  endl ;
*/
	cout << "length of 2nd step reductions = " << localKeyCount1 << endl ;

	MPI_Barrier(MPI_COMM_WORLD) ;

	int* step2Counts = new int [nprocs] ;

		MPI_Gather(&localKeyCount1, 1, MPI_INT, step2Counts, 1, MPI_INT, 0, MPI_COMM_WORLD) ;
		MPI_Barrier(MPI_COMM_WORLD) ;
		int maxlen1 = 0 ;


	int* finalRecvBufferKeys, *finalRecvBufferVals, *finalRecvCounts, *finalRecvDispls ;

	if (rank == 0) {
		for (int k = 0 ; k < nprocs ; k++)
						if (step2Counts[k] > maxlen1)
							maxlen1 = step2Counts[k] ;

		finalRecvBufferKeys = new int [nprocs*maxlen1] ;
		for (int i = 0 ; i < nprocs*maxlen1 ; i++)
			finalRecvBufferKeys[i] = -1 ;

		finalRecvBufferVals = new int [nprocs*maxlen1] ;
		for (int i = 0 ; i < nprocs*maxlen1 ; i++)
			finalRecvBufferVals[i] = -1 ;

		finalRecvCounts = new int [nprocs] ;
		finalRecvDispls = new int [nprocs] ;
		for (int i = 0 ; i < nprocs ; i++){
			finalRecvCounts[i] = maxlen1 ;
			finalRecvDispls[i] = i*maxlen1 ;
		}
	}
	MPI_Barrier(MPI_COMM_WORLD) ;
	MPI_Gatherv(localKeys1, localKeyCount1, MPI_INT, finalRecvBufferKeys,
			finalRecvCounts, finalRecvDispls, MPI_INT, 0, MPI_COMM_WORLD) ;

	MPI_Gatherv(localReduceSumsArr1, localKeyCount1, MPI_INT, finalRecvBufferVals,
			finalRecvCounts, finalRecvDispls, MPI_INT, 0, MPI_COMM_WORLD) ;
	MPI_Barrier(MPI_COMM_WORLD) ;

	if (rank == 0) {

		ofstream myfile("Output_Task2") ;
		if (myfile.is_open()){
			myfile << "Key \t" << "Sumreduction" << endl ;
			for (int i = 0 ; i < 10000 ; i++)
				if (finalRecvBufferKeys[i] != -1)
					myfile << finalRecvBufferKeys[i] << "\t" << finalRecvBufferVals[i] << endl ;
			// myfile << "node " << i << "\t -> " << initialPageRank[i] << endl ;
			myfile.close() ;
		}
	}



	MPI_Barrier(MPI_COMM_WORLD) ;
	MPI_Finalize();
	return 0;
}
