/*
 * GP_InfMatch.cpp
 *
 *  Created on: Apr 17, 2015
 *      Author: dimitris
 */


#include "GP_InfMatch.hpp"

#include "Groups.hpp"
#include "UserRatings.hpp"
#include "GroupRatings.hpp"
#include "InfMatchModel.hpp"



void GP_InfMatch::predictRatings(string ratingsFile, string groupsFile, string groupRatingsFile) {

	Groups groups;
	groups.read(groupsFile);

	UserRatings userRatings;
	userRatings.readRatings(ratingsFile);

	GroupRatings groupRatings;
	groupRatings.readGroupRatings(groupRatingsFile + "_train");

	// train information matching model
	InfMatchModel model;
	model.setInputData(groups, userRatings, groupRatings);
	model.relevanceThreshold = expSet.relevanceThreshFraction * expSet.maxRating ;
	model.learnParameters();
	model.setDescriptionVectors();

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

			float predicted = model.getGroupRelevanceEstimate(groupID, itemID) * expSet.maxRating; // multiply by maximum rating
			predR[groupID][itemID] = predicted;

		}

	}

}


