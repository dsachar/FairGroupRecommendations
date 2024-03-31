/*
 * GP_Agg_Prof.cpp
 *
 *  Created on: Mar 31, 2015
 *      Author: dimitris
 */

#include "GP_Agg_Prof.hpp"

#include "Groups.hpp"
#include "UserRatings.hpp"
#include "MatrixFactorization.hpp"

#include <cfloat>


void GP_Agg_Prof::setPredictionType(GroupAggType type) {
	this->groupPredType = type;
}

void GP_Agg_Prof::setPredictionType(int type) {
	this->groupPredType = type;
}



GP_Agg_Prof::GP_Agg_Prof(ExpSetting& expSet, int type) : GroupPredictor(expSet) {
	this->groupPredType = type;
}


void GP_Agg_Prof::predictRatings(string ratingsFile, string groupsFile, string groupRatingsFile) {

	Groups groups;
	groups.read(groupsFile);

	UserRatings userRatings;
	userRatings.readRatings(ratingsFile);


	int virtualUserIDStart = 10000000;


	// construct combined profiles for the groups
	for (auto& groupItemPair : evalR){ // for each group
		int groupID = groupItemPair.first;

		int virtualUserID = virtualUserIDStart + groupID; // virtual user representing the group
		userRatings.userIDs.insert(virtualUserID);


//		for (int itemID : itemIDs) {
//			if (trainR[groupID].count(itemID)) {
//				continue; // ignore items used in training
//			}

		for (auto& itemRatePair : groupItemPair.second) { // for each item
			int itemID = itemRatePair.first;

			float combined = 0;
			int numRatings = 0;

			if (groupPredType == MIN) {
				combined = numeric_limits<float>::infinity();
				for (auto& userWeightPair : groups.groupUserWeights[groupID]) { // for each user in group
					int userID = userWeightPair.first;

					if (userRatings.ratings[userID].count(itemID)) {
						numRatings++;
						if (combined > userRatings.ratings[userID][itemID]) {
							combined = userRatings.ratings[userID][itemID];
						}
					}
				}
			}
			else if (groupPredType == MAX) {
				combined = - numeric_limits<float>::infinity();
				for (auto& userWeightPair : groups.groupUserWeights[groupID]) { // for each user in group
					int userID = userWeightPair.first;

					if (userRatings.ratings[userID].count(itemID)) {
						numRatings++;
						if (combined < userRatings.ratings[userID][itemID]) {
							combined = userRatings.ratings[userID][itemID];
						}
					}
				}
			}
			else if (groupPredType == PRD) {
				combined = 1;
				for (auto& userWeightPair : groups.groupUserWeights[groupID]) { // for each user in group
					int userID = userWeightPair.first;

					if (userRatings.ratings[userID].count(itemID)) {
						numRatings++;
						combined *= userRatings.ratings[userID][itemID] / userRatings.maxRating;
					}
				}
				combined = combined * userRatings.maxRating; // scale back from [0,1]
			}
			else { // type == AVG or BEH
				float sumWeights = 0;
				for (auto& userWeightPair : groups.groupUserWeights[groupID]) { // for each user in group
					int userID = userWeightPair.first;
					float weight;

					if (userRatings.ratings[userID].count(itemID)) {
						numRatings++;
						if (groupPredType == AVG) {
							weight = 1;
						}
						else { // type == BEH
							weight = userWeightPair.second;
						}
						sumWeights += weight;
						combined += weight * userRatings.ratings[userID][itemID];
					}
				}
				if (sumWeights == 0) {
					combined = 0;
				}
				else {
					combined /= sumWeights;
				}
			}

			if (numRatings == 0) {
				continue;
			}

			userRatings.ratings[virtualUserID][itemID] = combined; // construct combined profile
		}
	}


	// factorize userRatings matrix
	MatrixFactorization mf;
	mf.setInputData(userRatings.ratings, userRatings.userIDs, userRatings.itemIDs, userRatings.ratingsFile + "TYPE" + to_string(groupPredType));
	mf.SGD_train(FACTORS, 0.005, 0.001, expSet.numEpochs);


	// create predictions
	for (auto& groupItemPair : evalR){ // for each group
		int groupID = groupItemPair.first;
		int virtualUserID = virtualUserIDStart + groupID; // virtual user representing the group

		for (auto& itemRatePair : groupItemPair.second) { // for each item
			int itemID = itemRatePair.first;
			float actual = itemRatePair.second;

			float predicted = mf.getEstimate(virtualUserID, itemID);

			predR[groupID][itemID] = predicted;
		}
	}

}


string const GP_Agg_Prof::methodName() {
	if (groupPredType == MIN) {
		return "AGG-PROF-MIN";
	}
	else if (groupPredType == AVG) {
		return "AGG-PROF-AVG";
	}
	else if (groupPredType == WEI) {
		return "AGG-PROF-BEH";
	}
	else if (groupPredType == MAX) {
		return "AGG-PROF-MAX";
	}
	else if (groupPredType == PRD) {
		return "AGG-PROF-PRD";
	}
	return "";
}
