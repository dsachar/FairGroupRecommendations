/*
 * UserRatings.cpp
 *
 *  Created on: Mar 20, 2015
 *      Author: dimitris
 */

#include "UserRatings.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cmath>
#include <queue>
#include <limits>
#include <random>





UserRatings::UserRatings() {
}

void UserRatings::readRatings(string ratingsFile){
	this->ratingsFile = ratingsFile;
	ifstream rin(ratingsFile.c_str());

	maxRating = - numeric_limits<float>::infinity();
	numUserRatings = 0;
	string line;
	while (getline(rin, line)) {
	    istringstream iss(line);
	    int user, item;
	    float rate;
		if ( !(iss >> user) ) {
			break;
		}
		userIDs.insert(user);
		if ( !(iss >> item) ) {
			break;
		}
		itemIDs.insert(item);
		if ( !(iss >> rate) ) {
			break;
		}
		ratings[user][item] = rate;
		if (rate > maxRating) {
			maxRating = rate;
		}
		numUserRatings++;
	}
	rin.close();

	numUsers = ratings.size();
	numItems = itemIDs.size();

}


void UserRatings::printRatings(){
	for (auto& userItemPair : ratings) {
		int user = userItemPair.first;
		for (auto& itemRatingPair : userItemPair.second) {
			int item = itemRatingPair.first;
			float rate = itemRatingPair.second;
			cout << "user " << user << " item " << item << " rate " << rate << endl;
		}
	}
}


pair<float, int> UserRatings::computeCosSimilarity(int userA, int userB, set<int> items) {
	set<int> itemsA, itemsB;
//	cout << "userA: " << userA << " userB: " << userB << endl;

	int numCommonItems;

	map<int,float> ratingsA, ratingsB;

	map<int,map<int,float>>::iterator it = ratings.find(userA);
	if (it != ratings.end()) {
		ratingsA = it->second;
	}
	else {
		return make_pair(0,-1);
	}
	it = ratings.find(userB);
	if (it != ratings.end()) {
		ratingsB = it->second;
	}
	else {
		return make_pair(0,-1);
	}

	float meanRatingA = 0;
	for (auto& itemRatingPair : ratingsA) {
		int item = itemRatingPair.first;
		if (!items.empty() && items.find(item) == items.end()) continue; // skip items not in the given set<int> items
		float rating = itemRatingPair.second;
		itemsA.insert(item);
		meanRatingA += rating;
	}
	meanRatingA /= ratingsA.size();

	float sumSqRatingsA = 0;
	for (auto& itemRatingPair : ratingsA) {
		int item = itemRatingPair.first;
		if (!items.empty() && items.find(item) == items.end()) continue; // skip items not in the given set<int> items
		float rating = itemRatingPair.second;
		sumSqRatingsA += (rating - meanRatingA) * (rating - meanRatingA);
	}

//	cout << "userA rated " << itemsA.size() << " items with meanRating: " << meanRatingA << " and sumSqRatings: " << sumSqRatingsA << endl;

	float meanRatingB = 0;
	for (auto& itemRatingPair : ratingsB) {
		int item = itemRatingPair.first;
		if (!items.empty() && items.find(item) == items.end()) continue; // skip items not in the given set<int> items
		float rating = itemRatingPair.second;
		itemsB.insert(item);
		meanRatingB += rating;
	}
	meanRatingB /= ratingsB.size();


	float sumSqRatingsB = 0;
	for (auto& itemRatingPair : ratingsB) {
		int item = itemRatingPair.first;
		if (!items.empty() && items.find(item) == items.end()) continue; // skip items not in the given set<int> items
		float rating = itemRatingPair.second;
		sumSqRatingsB += (rating - meanRatingB) * (rating - meanRatingB);
	}

//	cout << "userB rated " << itemsB.size() << " items with meanRating: " << meanRatingB << " and sumSqRatings: " << sumSqRatingsB << endl;

	set<int> commonItems;
	set_intersection(itemsA.begin(), itemsA.end(), itemsB.begin(), itemsB.end(), std::inserter(commonItems,commonItems.begin()));
	numCommonItems = commonItems.size();

	float prod = 0;
	for (int item : commonItems) {
		prod += (ratingsA.find(item)->second - meanRatingA) * (ratingsB.find(item)->second - meanRatingB);
	}

//	cout << "prod: " << prod << " over " << commonItems.size() << " items" << endl;

	float cosine = prod / sqrt(sumSqRatingsA * sumSqRatingsB);
//	cout << "cosine: " << cosine << endl;

	return make_pair(cosine, numCommonItems);

}


void UserRatings::computeAllPairsSimilarities() {

	ofstream fout;
	fout.open( ratingsFile + ".sims", ios::app);

	vector<float> sims;
	vector<int> numCommonItems;
	for (int oneUser : userIDs) {
		for (int otherUser : userIDs) {
			if (otherUser <= oneUser) continue;
			float sim;
			int commonItems;
			tie(sim, commonItems) = computeCosSimilarity(oneUser, otherUser);
			sims.push_back(sim);
			numCommonItems.push_back(commonItems);
		}
	}

	for (auto val : sims) {
		fout << val << " ";
	}
	fout << endl;

	for (auto val : numCommonItems) {
		fout << val << " ";
	}
	fout << endl;
	fout.close();
}



template <class T, class S, class C>
    S& Container(priority_queue<T, S, C>& q) {
        struct HackedQueue : private priority_queue<T, S, C> {
            static S& Container(priority_queue<T, S, C>& q) {
                return q.*&HackedQueue::c;
            }
        };
    return HackedQueue::Container(q);
}

struct GreaterThan
{
	bool operator()(const pair<int,float>& lhs, const pair<int,float>& rhs)
	{
	  return lhs.second > rhs.second;
	}
};

set<int> UserRatings::findNeighborhood(int userID, int k) {
	set<int> neighbors;

    priority_queue<pair<int,float>, vector<pair<int,float >>, GreaterThan> topK;


	for (auto& userItemPair : ratings) {
		int otherUser = userItemPair.first;
		if (otherUser == userID) continue;
		float sim;
		int numCommonItems;
		tie(sim, numCommonItems) = computeCosSimilarity(userID, otherUser);
		if (topK.size() < k) {
			topK.push(make_pair(otherUser, sim));
		}
		else if (sim > topK.top().second) {
			topK.pop();
			topK.push(make_pair(otherUser, sim));
		}
	}

	for (auto entry: Container(topK)) {
		neighbors.insert(entry.first);
		cout << "neighbor " << entry.first << " with sim: " << entry.second << endl;
	}

	return neighbors;
}


set<int> UserRatings::findSimilar(int seedUser, int k, set<int> items) {
	set<int> selectedUsers;

	selectedUsers.insert(seedUser);

//	cout << "items size " << items.size() << endl;

	for (int iter=2; iter<=k; iter++) {
		int bestUser = -1;
		float bestSim = - numeric_limits<float>::infinity();
		for (auto& userItemPair : ratings) {
			int otherUser = userItemPair.first;
			if (selectedUsers.find(otherUser) != selectedUsers.end()) continue;

			for (int user : selectedUsers) {
				float sim;
				int numCommonItems;
				tie(sim, numCommonItems) = computeCosSimilarity(user, otherUser, items);
				if (sim > bestSim) {
					bestSim = sim;
					bestUser = otherUser;
				}
			}
		}
		if (bestUser == -1) break;
//		cout << "most similar " << bestUser << " with sim: " << bestSim << endl;
		selectedUsers.insert(bestUser);
	}

	return selectedUsers;
}


set<int> UserRatings::findDissimilar(int seedUser, int k, set<int> items) {
	set<int> selectedUsers;

	selectedUsers.insert(seedUser);


	for (int iter=2; iter<=k; iter++) {
		int worstUser = -1;
		float worstSim = numeric_limits<float>::infinity();
		for (auto& userItemPair : ratings) {
			int otherUser = userItemPair.first;
			if (selectedUsers.find(otherUser) != selectedUsers.end()) continue;

			float groupSim = - numeric_limits<float>::infinity();
			for (int member : selectedUsers) {
				float sim;
				int numCommonItems;
				tie(sim, numCommonItems) = computeCosSimilarity(member, otherUser, items);
				if (sim > groupSim) {
					groupSim = sim;
				}
			}
			if (groupSim < worstSim) {
				worstSim = groupSim;
				worstUser = otherUser;
			}
		}
		cout << "least similar " << worstUser << " with sim: " << worstSim << endl;
		selectedUsers.insert(worstUser);
	}

	return selectedUsers;
}


struct LessThan
{
	bool operator()(const pair<int,float>& lhs, const pair<int,float>& rhs)
	{
	  return lhs.second < rhs.second;
	}
};


set<int> UserRatings::findFarhood(int userID, int k) {
	set<int> strangers;

    priority_queue<pair<int,float>, vector<pair<int,float >>, LessThan> topK;


	for (auto& userItemPair : ratings) {
		int otherUser = userItemPair.first;
		if (otherUser == userID) continue;
		float sim;
		int numCommonItems;
		tie(sim, numCommonItems) = computeCosSimilarity(userID, otherUser);
		if (topK.size() < k) {
			topK.push(make_pair(otherUser, sim));
		}
		else if (sim < topK.top().second) {
			topK.pop();
			topK.push(make_pair(otherUser, sim));
		}
	}

	for (auto entry: Container(topK)) {
		strangers.insert(entry.first);
		cout << "stranger " << entry.first << " with sim: " << entry.second << endl;
	}

	return strangers;
}

set<int> UserRatings::selectRandomUsers(int k) {
	set<int> selectedUsers;

	random_device rd;
	mt19937 mt_rand(rd());
//	if (globalRandomSeed != -1) {
//		mt_rand.seed(globalRandomSeed);
//	}

	uniform_int_distribution<> unif(0, userIDs.size()-1);


	for (int i=0; i<k; i++) {
		set<int>::const_iterator it = userIDs.begin();
		int rnd = unif(mt_rand);
		advance(it, rnd); // pick the rnd-th user
		selectedUsers.insert(*it);
	}

	return selectedUsers;
}


