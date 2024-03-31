/*
 * SGD_UserRatings.hpp
 *
 *  Created on: Mar 27, 2015
 *      Author: dimitris
 */

#ifndef SRC_MATRIXFACTORIZATION_HPP_
#define SRC_MATRIXFACTORIZATION_HPP_


#include "common.hpp"


#include <vector>
#include <cassert>
#include <tuple>
#include <set>
#include <map>

using namespace std;



class MatrixFactorization {
public:
	vector<MatrixEntry> entriesList;

	string inputFile;
	string modelFile;

	set<int> leftIDs; // users
	set<int> rightIDs; // items


	int K; // number of factors
	map<int, Colvec> leftMatrix; // users
	map<int, Colvec> rightMatrix; // items

	double mu;
	map<int, double> leftBias;
	map<int, double> rightBias;

	void setInputData(map<int,map<int,float>>& inputMatrix, set<int>& leftIDs, set<int>& rightIDs, string inputFile);


	void SGD_train(int K=10, double eta=0.0001, double lambda=0.01, int epochs=100);
	void GD_train(int K=10, double eta=0.0001, double lambda=0.01, int epochs=100);

	double objectiveFunction(double lambda);
	double getSSE();
	double getRMSE();

	void initializeMatrices(double val);

	double getEstimate(int leftID, int rightID);
	double applyLogistic(double value);

	bool readMatrices();
	void writeMatrices();

};



#endif /* SRC_MATRIXFACTORIZATION_HPP_ */
