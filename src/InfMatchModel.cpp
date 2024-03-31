/*
 * InfMatchModel.cpp
 *
 *  Created on: Apr 17, 2015
 *      Author: dimitris
 */

#include <cmath>
#include <cfloat>

#include "InfMatchModel.hpp"


void InfMatchModel::setInputData(Groups& groups, UserRatings& userRatings, GroupRatings& groupRatings) {
	userIDs = userRatings.userIDs;
	itemIDs = userRatings.itemIDs;
	this->userRatings = userRatings;

	// get user ratings
	for (auto& leftRightPair : userRatings.ratings) {
		int userID = leftRightPair.first;
		for (auto& rightRatingPair : leftRightPair.second) {
			int itemID = rightRatingPair.first;
			float rate = rightRatingPair.second;
			MatrixEntry r(userID, itemID, rate);
			this->userRatingsList.push_back(r);
		}
	}

	// get group ratings
	for (auto& leftRightPair : groupRatings.ratings) {
		int groupID = leftRightPair.first;
		for (auto& rightRatingPair : leftRightPair.second) {
			int itemID = rightRatingPair.first;
			float rate = rightRatingPair.second;
			MatrixEntry r(groupID, itemID, rate);
			this->groupRatingsList.push_back(r);
		}
	}

	// get group info
	for (auto& groupUserPair : groups.groupUserWeights) {
		int groupID = groupUserPair.first;
		set<int> userIDs;
		for (auto& userWeightPair : groupUserPair.second) {
			int userID = userWeightPair.first;
			userIDs.insert(userID);
		}
		groupUserIDs[groupID] = userIDs;
	}

}


void InfMatchModel::learnParameters() {

	map<int, int> u_count; // userID:num of ratings
	map<int, int> i_count; // itemID:num of ratings

	for (int userID : userIDs) {
		u_lam_0[userID] = 0;
		u_lam_1[userID] = 0;
		u_count[userID] = 0;
		u_p[userID] = 0;
	}

	for (int itemID : itemIDs) {
		i_lam_0[itemID] = 0;
		i_lam_1[itemID] = 0;
		i_count[itemID] = 0;
		i_p[itemID] = 0;
	}

	for (auto& matrixEntry : userRatingsList) {
		int userID = get<0>(matrixEntry);
		int itemID = get<1>(matrixEntry);
		float rating = get<2>(matrixEntry);

		u_count[userID]++;
		i_count[itemID]++;

		if (rating >= relevanceThreshold) {
			u_lam_1[userID] += rating;
			i_lam_1[itemID] += rating;
			u_p[userID] += 1.0;
			i_p[itemID] += 1.0;
		}
		else {
			u_lam_0[userID] += rating;
			i_lam_0[itemID] += rating;
		}
	}

	for (int userID : userIDs) {
//		u_lam_0[userID] /= (itemIDs.size() - u_p[userID]); // was (u_count[userID] - u_p[userID])
		if (u_count[userID] - u_p[userID] != 0) {
			u_lam_0[userID] /= (u_count[userID] - u_p[userID]);
		}
		if (u_p[userID] != 0) {
			u_lam_1[userID] /= u_p[userID];
		}
//		u_p[userID] /= itemIDs.size(); // was u_count[userID]
		u_p[userID] /= u_count[userID];

//		cout << "userID=" << userID << " u_lam_0=" << u_lam_0[userID] << " u_lam_1=" << u_lam_1[userID] << " u_p=" << u_p[userID] << endl;
	}

	for (int itemID : itemIDs) {
//		i_lam_0[itemID] /= (userIDs.size() - i_p[itemID]); // was (i_count[itemID] - i_p[itemID])
		if (i_count[itemID] - i_p[itemID] != 0) {
			i_lam_0[itemID] /= (i_count[itemID] - i_p[itemID]);
		}
		if (i_p[itemID] != 0) {
			i_lam_1[itemID] /= i_p[itemID];
		}
//		i_p[itemID] /= userIDs.size(); // was i_count[itemID]
		i_p[itemID] /= i_count[itemID];

//		cout << "itemID=" << itemID << " i_lam_0=" << i_lam_0[itemID] << " i_lam_1=" << i_lam_1[itemID] << " i_p=" << i_p[itemID] << endl;
	}

}


void InfMatchModel::setDescriptionVectors() {
	for (int userID : userIDs) {
		for (int itemID : itemIDs) {
			float rating = userRatings.ratings[userID][itemID];
//			cout << "userID=" << userID << " itemID=" << itemID << " rating=" << rating << endl;

			double A = exp(-u_lam_1[userID]) * pow(u_lam_1[userID], rating);
			double B = exp(-u_lam_0[userID]) * pow(u_lam_0[userID], rating);

			u_dec[userID][itemID] = ( A ) / ( u_p[userID] * A + (1-u_p[userID]) * B );

//			cout << "A=" << A << " B=" << B << " u_p=" << u_p[userID] << " u_dec=" << u_dec[userID][itemID] << endl;

			double C = exp(-i_lam_1[itemID]) * pow(i_lam_1[itemID], rating);
			double D = exp(-i_lam_0[itemID]) * pow(i_lam_0[itemID], rating);

			i_dec[itemID][userID] = ( C ) / ( i_p[itemID] * C + (1-i_p[itemID]) * D );

//			cout << "C=" << C << " D=" << D << " i_p=" << i_p[itemID] << " i_dec=" << i_dec[itemID][userID] << endl;

		}
	}

}


double InfMatchModel::getUserRelevanceEstimate(int userID, int itemID) {
	double relProb = 1.0;

	for (MatrixEntry r : userRatingsList) {
		int thisUserID = get<0>(r);
		int thisItemID = get<1>(r);
		float thisRating = get<2>(r);

		if (thisRating < relevanceThreshold) continue; // skip this rating

		double product = u_dec[userID][thisItemID] * i_dec[itemID][thisUserID];

		relProb *= product;

		if (relProb < 0.00001 || product < 0.00001) {
			relProb = 0;
			break;
		}

//		cout << " * " << product << "  " << relProb << endl;


//		if (isnan(product)) {
//			cout << "nan product found" << endl;
//		}
//		if (isnan(relProb)) {
//			cout << "relProb became nan" << endl;
//		}

	}

//	cout << "user:" << userID << " itemID:" << itemID << " relProb=" << relProb << endl;

	return relProb;
}


double InfMatchModel::getGroupRelevanceEstimate(int groupID, int itemID) {
	double minGroupRelProb = numeric_limits<double>::infinity();
	double avgGroupRelProb = 0;

	for (int userID : groupUserIDs[groupID]) {
		double relProb = getUserRelevanceEstimate(userID, itemID);
		avgGroupRelProb += relProb;
		if (relProb < minGroupRelProb) {
			minGroupRelProb = relProb;
		}
	}
	avgGroupRelProb /= groupUserIDs[groupID].size();

//	return avgGroupRelProb;
	return minGroupRelProb;
}
