/*
 * GP_Agg_Pred.cpp
 *
 *  Created on: Mar 31, 2015
 *      Author: dimitris
 */


#include "GP_Agg_Pred.hpp"

#include "Groups.hpp"
#include "UserRatings.hpp"
#include "MatrixFactorization.hpp"

#include <cfloat>


void GP_Agg_Pred::setPredictionType(int type) {
	this->groupPredType = type;
}


GP_Agg_Pred::GP_Agg_Pred(ExpSetting& expSet, int type) : GroupPredictor(expSet) {
	this->groupPredType = type;
}


void GP_Agg_Pred::predictRatings(string ratingsFile, string groupsFile, string groupRatingsFile) {

	Groups groups;
	groups.read(groupsFile);

	UserRatings userRatings;
	userRatings.readRatings(ratingsFile);

	MatrixFactorization mf;
	mf.setInputData(userRatings.ratings, userRatings.userIDs, userRatings.itemIDs, userRatings.ratingsFile);
	mf.SGD_train(FACTORS, 0.005, 0.001, expSet.numEpochs);


	for (auto& groupItemPair : evalR){ // for each group
		int groupID = groupItemPair.first;

//		for (int itemID : itemIDs) {
//			if (trainR[groupID].count(itemID)) {
//				continue; // ignore items used in training
//			}

		for (auto& itemRatePair : groupItemPair.second) { // for each item
			int itemID = itemRatePair.first;
			float actual = itemRatePair.second;

			float predicted = 0;

			if (groupPredType == MIN) {
				predicted = numeric_limits<float>::infinity();
				for (auto& userWeightPair : groups.groupUserWeights[groupID]) { // for each user in group
					int userID = userWeightPair.first;

					if (predicted > mf.getEstimate(userID, itemID) ) {
						predicted = mf.getEstimate(userID, itemID);
					}
				}
			}
			else if (groupPredType == MAX) {
				predicted = - numeric_limits<float>::infinity();
				for (auto& userWeightPair : groups.groupUserWeights[groupID]) { // for each user in group
					int userID = userWeightPair.first;

					if (predicted < mf.getEstimate(userID, itemID) ) {
						predicted = mf.getEstimate(userID, itemID);
					}
				}
			}
			else if (groupPredType == PRD) {
				predicted = 1;
				for (auto& userWeightPair : groups.groupUserWeights[groupID]) { // for each user in group
					int userID = userWeightPair.first;

					if (userRatings.ratings[userID].count(itemID)) {
						predicted *= userRatings.ratings[userID][itemID] / userRatings.maxRating;
					}
				}
				predicted = predicted * userRatings.maxRating; // scale back from [0,1]
			}
			else { // type == AVG or BEH
				float sumWeights = 0;
				for (auto& userWeightPair : groups.groupUserWeights[groupID]) { // for each user in group
					int userID = userWeightPair.first;
					float weight;

					if (groupPredType == AVG) {
						weight = 1;
					}
					else { // type == BEH
						weight = userWeightPair.second;
					}

					sumWeights += weight;
					predicted += weight * mf.getEstimate(userID, itemID);
				}
				if (sumWeights == 0) {
					predicted = 0;
				}
				else {
					predicted /= sumWeights;
				}
			}

			predR[groupID][itemID] = predicted;
		}
	}


}



string const GP_Agg_Pred::methodName() {
	if (groupPredType == MIN) {
		return "AGG-PRED-MIN";
	}
	else if (groupPredType == AVG) {
		return "AGG-PRED-AVG";
	}
	else if (groupPredType == WEI) {
		return "AGG-PRED-BEH";
	}
	else if (groupPredType == MAX) {
		return "AGG-PRED-MAX";
	}
	else if (groupPredType == PRD) {
		return "AGG-PRED-PRD";
	}
	return "";
}

