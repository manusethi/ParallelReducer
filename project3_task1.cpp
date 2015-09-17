//============================================================================
// Name        : project2check.cpp
// Author      : ms
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <cmath>
#include <string>
#include <omp.h>

using namespace std;

int main(int argc, char* argv[]) {
	// cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	ifstream graphInputFile ;
	// graphInputFile.open("facebook_combined.txt") ;
	graphInputFile.open(argv[1]) ;
	int numNodes = 0 ;
	int numEdges = 0 ;
	int a, b, maxab = 0 ;
	if (graphInputFile.is_open()){
		while(graphInputFile >> a >> b)
		{
			maxab = (a>b ? a : b) ;
			numNodes = (maxab>numNodes ? maxab : numNodes) ;
			numEdges += 1 ;
		}
		numNodes += 1 ;
	}
	else cout <<"not open "<< endl ;
	graphInputFile.close() ;
	cout << numNodes << "\t" << numEdges << endl ;
	vector<list<int> > graph(numNodes) ;
	cout << "size of graph is " << graph.size() << endl ;

	graphInputFile.open(argv[1]) ;
	int x, y ;
	while (graphInputFile >> x >> y)
	{

		graph.at(x).push_back(y) ;
		graph.at(y).push_back(x) ;
		// cout << x << "\t" << y << endl ;

	}
	graphInputFile.close() ;

	// cout << graph.at(1).size() << endl ;

	vector<double> pageRank(numNodes) ;
	vector<double>::iterator it ;
	vector<double> initialPageRank(numNodes) ;

	double dVal = 0.85 ;
	double errVal = 0.0 ;
	double toleranceVal = 1e-8 ;
	int nProcessors = omp_get_max_threads() ;
	cout << nProcessors << "\t processors threads" << endl ;
	omp_set_num_threads(nProcessors) ;

	// TODO: initialize pageRank to zero

	// initialize initialPageRank vector
	#pragma omp parallel for
	for (int i = 0 ; i < numNodes ; i++)
	{
		initialPageRank.at(i) = 1.0 / numNodes ;
	}

	int maxIter = 1000 ;
	int iterNumber = 0 ;
	while(iterNumber++ < maxIter)
	{
		#pragma omp parallel for
		for (int i = 0 ; i < numNodes ; i++)
		{
			/*int nThreads = omp_get_num_threads() ;
	   cout << nThreads << "\t threads" << endl ;*/
			for (list<int>::const_iterator iterator = graph.at(i).begin(), end = graph.at(i).end() ;
					iterator != end ; ++iterator)
			{
				pageRank.at(i) += initialPageRank.at(*iterator) / graph.at(*iterator).size() ;
			}
			pageRank.at(i) = (1.0-dVal)/numNodes + dVal*pageRank.at(i) ;

		}


		#pragma omp parallel for reduction(+:errVal)
		for (int i = 0 ; i < numNodes ; i++)
			errVal += fabs(pageRank.at(i)-initialPageRank.at(i)) ;
		errVal /= numNodes ;

		if (errVal < toleranceVal)
		{
			cout << "num of Iterations" << iterNumber << endl ;
			cout << "error is " << errVal << endl ;
			break ;

		}


		initialPageRank = pageRank ;
		//assign pageRank back to zero
		pageRank.assign(numNodes, 0) ; // check
		// cout << "page rank at 1 is " << initialPageRank.at(9) << endl ;

		// cout << "page rank at 1 is " << pageRank.at(1) << endl ;

	} // end while
	cout << "error is " << errVal << endl ;

	ofstream myfile("Output_Task1") ;
	if (myfile.is_open()){
		myfile << "node \t" << "pagerank" << endl ;
		for (int i = 0 ; i < numNodes ; i++)
			myfile << i << "\t" << initialPageRank[i] << endl ;
		myfile.close() ;
	}


	return 0;
}
