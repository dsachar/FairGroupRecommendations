/*
 * ResidualBehavior.cpp
 *
 *  Created on: Mar 20, 2015
 *      Author: dimitris
 */


#include "ResidualBehavior.hpp"

#include <cmath>
#include <fstream>

#include "Groups.hpp"
#include "UserRatings.hpp"
#include "GroupRatings.hpp"
#include "MatrixFactorization.hpp"



ResidualBehavior::ResidualBehavior(UserRatings& userRatings, GroupRatings& groupRatings, Groups& groups) : userRatings(userRatings), groupRatings(groupRatings), groups(groups) {
}


void ResidualBehavior::calculateResidualBehavior(){
	// factorize userRatings matrix to make predictions when necessary
	MatrixFactorization mf_u;
	mf_u.setInputData(userRatings.ratings, userRatings.userIDs, userRatings.itemIDs, userRatings.ratingsFile);
	mf_u.SGD_train(10, 0.005, 0.001, 5);


	for (auto& groupItemPair : groupRatings.ratings) { // for each group
		int groupID = groupItemPair.first;

		map<int, float> userWeights = groups.groupUserWeights[groupID];

		for (auto& userWeight : userWeights) { // for each user in group
			int userID = userWeight.first;

			float residualBehavior = 0;

			for (auto& itemRatePair : groupItemPair.second) { // for each item rated by group
				int itemID = itemRatePair.first;

				float rating;
				if (userRatings.ratings[userID].count(itemID)) { // get actual
					rating = userRatings.ratings[userID][itemID];
				}
				else { // predict
					rating = mf_u.getEstimate(userID, itemID);
				}

				residualBehavior += groupRatings.ratings[groupID][itemID] - rating;
			}
			residualBehavior /= groupItemPair.second.size();
			if (fabs(residualBehavior) < 1e-5) residualBehavior = 0; // account for rounding errors
			userGroupResidualBehavior[userID][groupID] = residualBehavior;
//			cout << "user " << user << " group " << group << " residualBehavior " << userGroupResidualBehavior[user][group] << endl;

		}

	}

	// fill with neutral behavior
//	for (auto& groupItemPair : groupRatings.ratings) {
//		int groupID = groupItemPair.first;
//
//		map<int, float> userWeights = groups.groupUserWeights[groupID];
//
//		for (int userID : groups.distinctUsers) {
//			if ( !userWeights.count(userID) ) { // if user not in group
//				userGroupResidualBehavior[userID][groupID] = 0; // set neutral behavior
//			}
//
//		}
//	}


}



void ResidualBehavior::writeResidualBehavior(string residualBehaviorFile) {
	ofstream rb_out;
	rb_out.open(residualBehaviorFile.c_str(), ios::out);
	for (auto& userGroupPair : userGroupResidualBehavior) {
		int user = userGroupPair.first;
		for (auto& GroupBehaviorPair : userGroupPair.second) {
			int group = GroupBehaviorPair.first;
			float residualBehavior = GroupBehaviorPair.second;
			rb_out << user << " " << group << " " << residualBehavior << endl;
		}
	}
	rb_out.close();
}
