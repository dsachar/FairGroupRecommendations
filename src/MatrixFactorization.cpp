/*
 * SGD_UserRatings.cpp
 *
 *  Created on: Mar 27, 2015
 *      Author: dimitris
 */


#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <random>
#include "MatrixFactorization.hpp"




void MatrixFactorization::setInputData(map<int,map<int,float>>& ratings, set<int>& leftIDs, set<int>& rightIDs, string inputFile){
	this->leftIDs = leftIDs;
	this->rightIDs = rightIDs;
	this->mu = 0;
	for (auto& leftRightPair : ratings) {
		int user = leftRightPair.first;
		for (auto& rightRatingPair : leftRightPair.second) {
			int item = rightRatingPair.first;
			float rate = rightRatingPair.second;
			MatrixEntry r(user, item, rate);
			this->entriesList.push_back(r);
			this->mu += rate;
		}
	}
	this->inputFile = inputFile;
	this->mu /= this->entriesList.size();
}


void MatrixFactorization::SGD_train(int K, double eta, double lambda, int epochs) {
	modelFile = inputFile +"_MDL_K" + to_string(K) + "_E" + to_string(eta) + "_L" + to_string(lambda) + "_I" + to_string(epochs) + "_SD" + to_string(globalRandomSeed);

	if (readMatrices()) { // read already trained model
		return;
	}

	this->K = K;
//	initializeMatrices(1.0);
//	initializeMatrices(1.0/K);
//	initializeMatrices(0.5/K);
	initializeMatrices(0.0);

	int epoch = 1;

	cerr << "K=" << K << " eta=" << eta << " lambda=" << lambda << " epochs=" << epochs << endl;

	random_device rd;
	mt19937 mt_rand(rd());
	if (globalRandomSeed != -1) {
		mt_rand.seed(globalRandomSeed);
	}

	while(epoch <= epochs) {
//		cerr << "\repoch " << epoch;
		shuffle(entriesList.begin(), entriesList.end(), mt_rand);

		for (MatrixEntry r : entriesList) {
			int leftID = get<0>(r);
			int rightID = get<1>(r);
			float rate = get<2>(r);
//			cout << "leftID " << leftID << " rightID " << rightID << " rate " << rate << endl;

//			double pred = innerProduct(leftMatrix[leftID], rightMatrix[rightID]);
			double pred = mu + leftBias[leftID] + rightBias[rightID] + innerProduct(leftMatrix[leftID], rightMatrix[rightID]);

//			double pred = applyLogistic ( innerProduct(leftMatrix[leftID], rightMatrix[rightID]) ); // WRONG: with logistic filter; change the DERIVATIVES

//			cout << "innerProd: " << pred << " rating: " << rate << endl;
			double error = pred - rate; // IMPORTANT

//			if (isnan(error)) {
//				double pred = innerProductDEBUG(leftMatrix[leftID], rightMatrix[rightID]);
//				cout << "(" << rate << ":" << pred << ") ";
//				writeMatrices();
//				exit(0);
//			}


			leftBias[leftID] -= 2 * eta * (error + lambda * leftBias[leftID]);
			rightBias[rightID] -= 2 * eta * (error + lambda * rightBias[rightID]);

			for (int f=0; f<K; f++) { // for each factor
//				if (abs(leftMatrix[leftID][f]) > 1) {
//					cout << "BEFORE leftID=" << leftID << " f=" << f << " val=" << leftMatrix[leftID][f] << endl;
//				}
//				if (abs(rightMatrix[rightID][f]) > 1) {
//					cout << "BEFORE rightID=" << rightID << " f=" << f << " val=" << rightMatrix[rightID][f] << endl;
//				}
				leftMatrix[leftID][f] -= 2 * eta * (error * rightMatrix[rightID][f] + lambda * leftMatrix[leftID][f]);
				rightMatrix[rightID][f] -= 2 * eta * (error * leftMatrix[leftID][f] + lambda * rightMatrix[rightID][f]);
//				if (abs(leftMatrix[leftID][f]) > 1) {
//					cout << "AFTER leftID=" << leftID << " f=" << f << " val=" << leftMatrix[leftID][f] << endl;
//				}
//				if (abs(rightMatrix[rightID][f]) > 1) {
//					cout << "AFTER rightID=" << rightID << " f=" << f << " val=" << rightMatrix[rightID][f] << endl;
//				}

			}
		}


		double obj = objectiveFunction(lambda);
		cerr << "\repoch " << epoch << ": objective=" << obj << "                      ";

//		double obj = objectiveFunction(lambda);
//		cout << "epoch " << epoch << " objective=" << obj << " rmse=" << getRMSE() << endl;
//		if (obj < 0.001) break;

		epoch++;
	}
	cerr << endl;
	double obj = objectiveFunction(lambda);
	cerr << "final: objective=" << obj << " rmse=" << getRMSE() << endl;

	writeMatrices();

}


void MatrixFactorization::GD_train(int K, double eta, double lambda, int epochs) {

	modelFile = inputFile +"_MDL_K" + to_string(K) + "_E" + to_string(eta) + "_L" + to_string(lambda) + "_I" + to_string(epochs) + "_SD" + to_string(globalRandomSeed);

//	if (readMatrices()) { // read already trained model
//		return;
//	}


	this->K = K;
//	initializeMatrices(1.0);
	initializeMatrices(1.0/K);
//	initializeMatrices(0.5/K);
//	initializeMatrices(0.0);

	int epoch = 1;

	while(epoch <= epochs) {


		epoch++;
	}
	cerr << ": ";
	double obj = objectiveFunction(lambda);
	cerr << "objective=" << obj << " rmse=" << getRMSE() << endl;

	writeMatrices();
}



double MatrixFactorization::objectiveFunction(double lambda) {
	double sse = getSSE();
	return sse + lambda * (norm(leftMatrix) + norm(rightMatrix) );
}


double MatrixFactorization::getSSE() {
	double sse = 0;
	for (MatrixEntry r : entriesList) {
		int leftID = get<0>(r);
		int rightID = get<1>(r);
		float rate = get<2>(r);

//		double pred = innerProduct(leftMatrix[leftID], rightMatrix[rightID]);
		double pred = mu + leftBias[leftID] + rightBias[rightID] + innerProduct(leftMatrix[leftID], rightMatrix[rightID]);

		double error = pred - rate;
//		cout << "(" << rate << ":" << pred << ") ";

		sse += error*error;
	}
//	cout << endl;
//	cout << "sse=" << sse << endl;

	return sse;
}


double MatrixFactorization::getRMSE() {
	return sqrt (getSSE() / entriesList.size());
}


void MatrixFactorization::initializeMatrices(double val) {

	random_device rd;
	mt19937 mt_rand(rd());
	if (globalRandomSeed != -1) {
		mt_rand.seed(globalRandomSeed);
	}

	uniform_real_distribution<> dis(0, 0.1);



	for (int leftID : leftIDs) {
		Colvec vec(K, val);
		for (int i=0; i<vec.size(); i++) {
			vec[i] += dis(mt_rand);
		}
		leftMatrix[leftID] = vec;

		leftBias[leftID] = dis(mt_rand);
	}

	for (int rightID : rightIDs) {
		Colvec vec(K, val);
		for (int i=0; i<vec.size(); i++) {
			vec[i] += dis(mt_rand);
		}
		rightMatrix[rightID] = vec;

		rightBias[rightID] = dis(mt_rand);
	}
}


double MatrixFactorization::getEstimate(int leftID, int rightID) {
	if (leftIDs.count(leftID) == 0) {
		cout << "could not find leftID " << leftID << endl;
	}

	if (rightIDs.count(rightID) == 0) {
		cout << "could not find rightID " << rightID << endl;
	}

//	double pred = innerProduct( leftMatrix[leftID], rightMatrix[rightID]);
	double pred = mu + leftBias[leftID] + rightBias[rightID] + innerProduct( leftMatrix[leftID], rightMatrix[rightID]);


	if (pred<0) pred = 0;
	if (pred>1) pred = 1;
	return pred;
//	return applyLogistic (innerProduct( leftMatrix[leftID], rightMatrix[rightID]) );
}


double MatrixFactorization::applyLogistic(double value) {
//	return value;
	return 1.0/ (1.0 + exp (- (value )));
//	return 1.0/ (1.0 + exp (- (value - 0.5)));
}

void MatrixFactorization::writeMatrices() {
	ofstream mout;
	mout.open(modelFile.c_str(), ios::out);

	mout << K << " " << leftIDs.size() << " " << rightIDs.size() << endl;

	for (auto& leftPair : leftMatrix) {
		int leftID = leftPair.first;
		mout << leftID;
		Colvec vec = leftPair.second;
		for (double factor : vec) {
			mout << " " << factor;
		}
		mout << endl;
	}

	for (auto& rightPair : rightMatrix) {
		int leftID = rightPair.first;
		mout << leftID;
		Colvec vec = rightPair.second;
		for (double factor : vec) {
			mout << " " << factor;
		}
		mout << endl;
	}

	mout.close();
}


bool MatrixFactorization::readMatrices() {
	ifstream min;
	min.open(modelFile.c_str(), ios::in);

	if (min.fail()) {
		return false;
	}

	int numLeftIDs, numRightIDs;
	string line;

	getline(min, line);
    istringstream iss(line);
    iss >> K;
    iss >> numLeftIDs;
    iss >> numRightIDs;

	for (int i=0; i<numLeftIDs; i++) {
		int leftID;
		min >> leftID;
		Colvec vec;
		for (int f=0; f<K; f++) {
			double val;
			min >> val;
			vec.push_back(val);
		}
		leftMatrix[leftID] = vec;
	}

	for (int i=0; i<numRightIDs; i++) {
		int rightID;
		min >> rightID;
		Colvec vec;
		for (int f=0; f<K; f++) {
			double val;
			min >> val;
			vec.push_back(val);
		}
		rightMatrix[rightID] = vec;
	}


	min.close();
	return true;
}

