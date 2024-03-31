/*
 * TriadModel.hpp
 *
 *  Created on: Apr 1, 2015
 *      Author: dimitris
 */

#ifndef SRC_TRIADMODEL_HPP_
#define SRC_TRIADMODEL_HPP_


#include <vector>
#include <cassert>
#include <tuple>
#include <set>
#include <map>

#include "Groups.hpp"
#include "UserRatings.hpp"
#include "GroupRatings.hpp"


using namespace std;


typedef vector<double> Colvec;
typedef tuple<int, int, float> MatrixEntry;


class TriadModel {
public:
	vector<MatrixEntry> userRatingsList;
	vector<MatrixEntry> groupRatingsList;

	set<int> userIDs; // users
	set<int> itemIDs; // items
	map<int, set<int>> groupUserIDs; // group:{userID}


	int K; // number of factors
	map<int, Colvec> userMatrix; // users
	map<int, Colvec> itemMatrix; // items

	map<int, float> userBehavior; // user:behavior


	void setInputData(Groups& groups, UserRatings& userRatings, GroupRatings& groupRatings);


	void SGD_train(int K=10, double eta=0.0001, double lambda=0.01, int epochs=100);

	double objectiveFunction(double lambda);
	double getSSE();
	double getRMSE();

	void initializeModel(double val);

	double getUserRatingEstimate(int userID, int itemID);
	double getGroupRatingEstimate(int groupID, int itemID);

	double getUserRatingsSSE();
	double getUserRatingsRMSE();

	double getGroupRatingsSSE();
	double getGroupRatingsRMSE();

	void readModel(string modelFile);
	void writeModel(string modelFile);

};


#endif /* SRC_TRIADMODEL_HPP_ */
