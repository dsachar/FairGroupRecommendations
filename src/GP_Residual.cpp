/*
 * GP_Residual.cpp
 *
 *  Created on: Mar 31, 2015
 *      Author: dimitris
 */


#include "GP_Residual.hpp"

#include "Groups.hpp"
#include "UserRatings.hpp"
#include "GroupRatings.hpp"
#include "MatrixFactorization.hpp"
#include "ResidualBehavior.hpp"



void GP_Residual::predictRatings(string ratingsFile, string groupsFile, string groupRatingsFile) {

	Groups groups;
	groups.read(groupsFile);

	UserRatings userRatings;
	userRatings.readRatings(ratingsFile);

	GroupRatings groupRatings;
	groupRatings.readGroupRatings(groupRatingsFile + ".train");

	ResidualBehavior residualBehavior(userRatings, groupRatings, groups);
	residualBehavior.calculateResidualBehavior();
//	residualBehavior.writeResidualBehavior(groupRatingsFile + "_train" + "_residual");
//	cout << "groups.distinctUsers size " << groups.distinctUsers.size() << " groups.groupIDs size " << groups.groupIDs.size() << endl;

	// factorize userRatings matrix
	MatrixFactorization mf_u;
	mf_u.setInputData(userRatings.ratings, userRatings.userIDs, userRatings.itemIDs, userRatings.ratingsFile);
	mf_u.SGD_train(FACTORS, 0.005, 0.001, expSet.numEpochs);

	// factorize residual behavior matrix
	MatrixFactorization mf_r;
	mf_r.setInputData(residualBehavior.userGroupResidualBehavior, groups.distinctUsers, groups.groupIDs, groupRatings.groupsFile);
	mf_r.SGD_train(FACTORS, 0.005, 0.001, 5*expSet.numEpochs);


	// create predictions
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

			int count = 0;
			for (auto& userWeightPair : groups.groupUserWeights[groupID]) { // for each user in group
				int userID = userWeightPair.first;

				predicted += mf_u.getEstimate(userID, itemID) + mf_r.getEstimate(userID, groupID);
				count++;
			}

			if (count == 0) {
				predicted = 0;
			}
			else {
				predicted /= count;
			}

			predR[groupID][itemID] = (predicted > userRatings.maxRating) ? userRatings.maxRating : predicted;
		}
	}

}



