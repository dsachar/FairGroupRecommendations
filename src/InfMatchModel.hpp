/*
 * InfMatchModel.hpp
 *
 *  Created on: Apr 17, 2015
 *      Author: dimitris
 */

#ifndef SRC_INFMATCHMODEL_HPP_
#define SRC_INFMATCHMODEL_HPP_


#include <set>
#include <map>
#include <vector>
#include <tuple>

#include "Groups.hpp"
#include "UserRatings.hpp"
#include "GroupRatings.hpp"


using namespace std;


class InfMatchModel {
public:
	vector<MatrixEntry> userRatingsList;
	vector<MatrixEntry> groupRatingsList;

	UserRatings userRatings;

	set<int> userIDs; // users
	set<int> itemIDs; // items
	map<int, set<int>> groupUserIDs; // group:{userID}


	float relevanceThreshold;

	// model parameters
	map<int, float> u_lam_0; // userID:lambda_0 (avg of non-rel ratings)
	map<int, float> u_lam_1; // userID:lambda_1 (avg of rel ratings)
	map<int, float> i_lam_0; // itemID:lambda_0
	map<int, float> i_lam_1; // itemID:lambda_1
	map<int, float> u_p; // user:mixture probability (#rel/#total)
	map<int, float> i_p; // item:mixture probability

	// description vectors
	map<int, map<int,double>> u_dec; // userID:itemID:value
	map<int, map<int,double>> i_dec; // itemID:userID:value


	void setInputData(Groups& groups, UserRatings& userRatings, GroupRatings& groupRatings);

	void learnParameters();
	void setDescriptionVectors();


	double getUserRelevanceEstimate(int userID, int itemID);
	double getGroupRelevanceEstimate(int groupID, int itemID);

};


#endif /* SRC_INFMATCHMODEL_HPP_ */
