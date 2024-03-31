/*
 * GP_Triad.cpp
 *
 *  Created on: Apr 1, 2015
 *      Author: dsachar
 */


#include "GP_Triad.hpp"

#include "Groups.hpp"
#include "UserRatings.hpp"
#include "GroupRatings.hpp"
#include "TriadModel.hpp"



void GP_Triad::predictRatings(string ratingsFile, string groupsFile, string groupRatingsFile) {

	Groups groups;
	groups.read(groupsFile);

	UserRatings userRatings;
	userRatings.readRatings(ratingsFile);

	GroupRatings groupRatings;
	groupRatings.readGroupRatings(groupRatingsFile + ".train");

	// learn triad model parameters
	TriadModel triad;
	triad.setInputData(groups, userRatings, groupRatings);
	triad.SGD_train(20, 0.01, 0.001, 10); // (FACTORS, 0.005, 0.001, expSet.numEpochs);


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

			float predicted = triad.getGroupRatingEstimate(groupID, itemID);
			predR[groupID][itemID] = predicted;

		}

	}

}

