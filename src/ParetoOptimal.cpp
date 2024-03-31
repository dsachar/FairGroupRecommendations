/*
 * ParetoOptimal.cpp
 *
 *  Created on: 10 Mar 2017
 *      Author: dimitris
 */

#include "ParetoOptimal.hpp"

#include <cfloat>
#include <random>
#include <algorithm>
#include <fstream>




ParetoOptimal::ParetoOptimal(){

}

ParetoOptimal::ParetoOptimal(UserRatings& ratings) : ur(ratings) {
	runMF();
}



void ParetoOptimal::clearItems() {
	items.clear();
	itemIDs.clear();
	poItems.clear();
	poItemIDs.clear();
	sharedRatings.clear();
}


void ParetoOptimal::setRandomItems(int num) {
	clearItems();

	if (num == -1) {
		itemIDs = ur.itemIDs;
	}
	else {
		vector<int> tmp(ur.itemIDs.begin(), ur.itemIDs.end());

		random_device rd;
		mt19937 mt_rand(rd());
		if (globalRandomSeed != -1) {
			mt_rand.seed(globalRandomSeed);
		}

		shuffle(tmp.begin(), tmp.end(), mt_rand);

		for (int i=0; i<num; i++) {
			itemIDs.insert(tmp[i]);
		}

	}
//	cout << "itemIDs size " << itemIDs.size() << endl;
//	cout << "itemIDs: " << vecToStr(vector<int>(itemIDs.begin(), itemIDs.end())) << endl;
}


void ParetoOptimal::setCommonRatedItems() {
	clearItems();

	bool first = true;
	for (int userID : users) {
		set<int> userItems;

		for (auto& itemRatingPair : ur.ratings[userID]) {
			int item = itemRatingPair.first;
			userItems.insert(item);
		}
		if (first) {
			first = false;
			itemIDs = userItems;
		}
		else {
			set<int> temp;
			set_intersection(itemIDs.begin(), itemIDs.end(), userItems.begin(), userItems.end(), inserter(temp,temp.begin()));
			itemIDs = temp;
		}
	}
}



void ParetoOptimal::setUsers(vector<int> users) {
	this->users = users;
}



void ParetoOptimal::runMF() {
	// do matrix factorization to predict missing ratings
	mf.setInputData(ur.ratings, ur.userIDs, ur.itemIDs, ur.ratingsFile);
//	mf.SGD_train(50, 0.005, 0.001, 3);
//	mf.SGD_train(100, 0.005, 0.001, 4);
//	mf.SGD_train(100, 0.02, 0.001, 20);
	mf.SGD_train(100, 0.02, 0.00, 20);
//	mf.SGD_train(10, 0.005, 0.001, 1);

}


void ParetoOptimal::retrieveRatings() {
	cout << "retrieveRatings" << endl;
	ofstream rout;
	rout.open("ratings.out", ios::out);

	for (int item : itemIDs) {
		vector<float> itemRatings;
		for (int user : users) {
//			cout << "user: " << user << endl;
			if (ur.ratings[user].find(item) != ur.ratings[user].end()) {
				itemRatings.push_back(ur.ratings[user][item]);
			}
			else {
				itemRatings.push_back(mf.getEstimate(user, item));
			}
		}
		sharedRatings[item] = itemRatings;
		items.push_back(Item(item, itemRatings));
//		cout << "for item " << item << " got " << vecToStr(itemRatings) << endl;
		rout << item << " " << vecToStr(itemRatings) << endl;
	}
//	cout << "all items are " << sharedRatings.size() << endl;
	rout.close();
}


//void ParetoOptimal::retrieveRandomItemSubset(int num) {
//	retrieveAllItems();
//	items.clear();
//	itemIDs.clear();
//
//	vector<int> allItems;
//	for (auto itemPair : sharedRatings) {
//		int item = itemPair.first;
//		allItems.push_back(item);
//	}
//
//
//	random_device rd;
//	mt19937 mt_rand(rd());
//	if (globalRandomSeed != -1) {
//		mt_rand.seed(globalRandomSeed);
//	}
//
//	shuffle(allItems.begin(), allItems.end(), mt_rand);
//	map<int, vector<float>> newRatings;
//	for (int i=0; i<num; i++) {
//		newRatings[allItems[i]] = sharedRatings[allItems[i]];
//		items.push_back(Item(allItems[i], sharedRatings[allItems[i]]));
//		itemIDs.insert(allItems[i]);
////		cout << allItems[i] << " ";
//	}
////	cout << endl;
//
//	sharedRatings = newRatings;
//	cout << "number of used ratings are " << sharedRatings.size() << endl;
//}


pair<float, int> ParetoOptimal::computeGroupSimilarity() {
	// similarity based on known ratings

	float worstPairwiseSim = numeric_limits<double>::infinity();

	float avgPairwiseSim = 0;

	int minNumCommonItems = numeric_limits<int>::infinity();

	int N = users.size();
	for (int i = 0; i<N; i++) {
		int userA = users[i];
		for (int j = i+1; j<N; j++) {
			int userB = users[j];
			float sim;
			int numCommonItems;
			tie(sim, numCommonItems) = ur.computeCosSimilarity(userA, userB);
			if (numCommonItems < minNumCommonItems) {
				minNumCommonItems = numCommonItems;
			}
			avgPairwiseSim += sim;
			if (sim < worstPairwiseSim) {
				worstPairwiseSim = sim;
			}
		}
	}

	avgPairwiseSim /= 0.5*N*(N-1);
//	cout << "avgPairwiseSim: " << avgPairwiseSim << endl;
	cout << "worstPairwiseSim: " << worstPairwiseSim << endl;
//	cout << "minNumCommonItems: " << minNumCommonItems << endl;

	return make_pair(worstPairwiseSim, minNumCommonItems);
}


float ParetoOptimal::computeGroupSimilarityOnSelectedItems() {
	// similarity based on predicted ratings of selected items

	float worstPairwiseSim = numeric_limits<double>::infinity();
	float bestPairwiseSim = -numeric_limits<double>::infinity();
	float avgPairwiseSim = 0;


	int N = users.size();
	for (int i = 0; i<N; i++) {
		int userA = users[i];
		for (int j = i+1; j<N; j++) {
			int userB = users[j];
			float sim = computeCosSimilarity(userA, userB);

			avgPairwiseSim += sim;
			if (sim < worstPairwiseSim) {
				worstPairwiseSim = sim;
			}
			if (sim > bestPairwiseSim) {
				bestPairwiseSim = sim;
			}
		}
	}

	avgPairwiseSim /= 0.5*N*(N-1);
	cout << "avgPairwiseSim: " << avgPairwiseSim << endl;
	cout << "bestPairwiseSim: " << bestPairwiseSim << endl;
	cout << "worstPairwiseSim: " << worstPairwiseSim << endl;

	return worstPairwiseSim;
}


float ParetoOptimal::computeCosSimilarity(int userA, int userB) {
	set<int> itemsA, itemsB;
//	cout << "userA: " << userA << " userB: " << userB << endl;


	map<int,float> ratingsA, ratingsB;

	for (int item : itemIDs) {
		float ratingA, ratingB;

		if (ur.ratings[userA].find(item) != ur.ratings[userA].end()) {
			ratingA = ur.ratings[userA][item];
		}
		else {
			ratingA = mf.getEstimate(userA, item);
		}

		ratingsA[item] = ratingA;

		if (ur.ratings[userB].find(item) != ur.ratings[userB].end()) {
			ratingB = ur.ratings[userB][item];
		}
		else {
			ratingB = mf.getEstimate(userB, item);
		}

		ratingsB[item] = ratingB;

	}


	float meanRatingA = 0;
	for (auto& itemRatingPair : ratingsA) {
		int item = itemRatingPair.first;
		float rating = itemRatingPair.second;
		itemsA.insert(item);
		meanRatingA += rating;
	}
	meanRatingA /= ratingsA.size();

	float sumSqRatingsA = 0;
	for (auto& itemRatingPair : ratingsA) {
		int item = itemRatingPair.first;
		float rating = itemRatingPair.second;
		sumSqRatingsA += (rating - meanRatingA) * (rating - meanRatingA);
	}

//	cout << "userA rated " << itemsA.size() << " items with meanRating: " << meanRatingA << " and sumSqRatings: " << sumSqRatingsA << endl;

	float meanRatingB = 0;
	for (auto& itemRatingPair : ratingsB) {
		int item = itemRatingPair.first;
		float rating = itemRatingPair.second;
		itemsB.insert(item);
		meanRatingB += rating;
	}
	meanRatingB /= ratingsB.size();


	float sumSqRatingsB = 0;
	for (auto& itemRatingPair : ratingsB) {
		int item = itemRatingPair.first;
		float rating = itemRatingPair.second;
		sumSqRatingsB += (rating - meanRatingB) * (rating - meanRatingB);
	}

//	cout << "userB rated " << itemsB.size() << " items with meanRating: " << meanRatingB << " and sumSqRatings: " << sumSqRatingsB << endl;

	float prod = 0;
	for (int item : itemIDs) {
		prod += (ratingsA.find(item)->second - meanRatingA) * (ratingsB.find(item)->second - meanRatingB);
	}

//	cout << "prod: " << prod << " over " << commonItems.size() << " items" << endl;

	float cosine = prod / sqrt(sumSqRatingsA * sumSqRatingsB);
//	cout << "cosine: " << cosine << endl;

	return cosine;

}



void ParetoOptimal::computeAllPairsSimilarities() {

	ofstream fout;
	fout.open( ur.ratingsFile + "_sample.sims", ios::app);

	vector<float> sims;
	for (int oneUser : ur.userIDs) {
		for (int otherUser : ur.userIDs) {
			if (otherUser <= oneUser) continue;
			float sim = computeCosSimilarity(oneUser, otherUser);
			sims.push_back(sim);
		}
	}

	for (auto val : sims) {
		fout << val << " ";
	}
	fout << endl;
	fout.close();
}


void ParetoOptimal::sampleAllPairsSimilarities() {

	ofstream fout;
	fout.open( ur.ratingsFile + "_sample.sims", ios::app);

	vector<float> sims;

	random_device rd;
	mt19937 mt_rand(rd());
	if (globalRandomSeed != -1) {
		mt_rand.seed(globalRandomSeed);
	}
	uniform_int_distribution<> unif(1, ur.numUsers);

	for (int i=0; i<100000; i++) {
		int userA = unif(mt_rand);
		int userB = unif(mt_rand);
		float sim = computeCosSimilarity(userA, userB);
		sims.push_back(sim);
	}


	for (auto val : sims) {
		fout << val << " ";
	}
	fout << endl;
	fout.close();
}



void ParetoOptimal::createGroupOLD(int numUsers, int groupType) {
	set<int> users;

	if (groupType == 0) {
		users = ur.selectRandomUsers(numUsers);
		vector<int> group(users.begin(), users.end());
		setUsers(group);
		computeGroupSimilarity();
	}
	else {
		random_device rd;
		mt19937 mt_rand(rd());
		if (globalRandomSeed != -1) {
			mt_rand.seed(globalRandomSeed);
		}
		uniform_int_distribution<> unif(1, ur.numUsers);

		bool acceptGroup = false;

		while (!acceptGroup) {
			int seedUser = unif(mt_rand);

			users = ur.findSimilar(seedUser, numUsers);
			vector<int> group(users.begin(), users.end());
			setUsers(group);
			float groupSim;
			int minNumCommonItems;
			tie(groupSim, minNumCommonItems) = computeGroupSimilarity();

//			if (groupType == 1 && groupSim > 0.032 && groupSim < 0.043) { // low similarity
//				acceptGroup = true;
//			}
//			else if (groupType == 2 && groupSim > 0.043) { // high similarity
//				acceptGroup = true;
//			}
			if (groupType == 1 && groupSim > 0.032) { // similarity threshold
				acceptGroup = true;
			}
		}
	}


}


void ParetoOptimal::createGroup(int numUsers, int groupType) {
	set<int> users;


	bool acceptGroup = false;

	while (!acceptGroup) {

		users = ur.selectRandomUsers(numUsers);
		vector<int> group(users.begin(), users.end());
		setUsers(group);
		float groupSim = computeGroupSimilarityOnSelectedItems();

		if (groupType == 0) {
			acceptGroup = true;
		}
		else if (groupType == 1 && groupSim > 0.5) {
			acceptGroup = true;
		}
	}


}




void ParetoOptimal::createGroupNEW(int numUsers, int groupType) {
	set<int> group;

	if (groupType == 0) {
		group = ur.selectRandomUsers(numUsers);

	}

	if (groupType == 1) { // similar
		group = ur.selectRandomUsers(1);

		for (int iter=1; iter<numUsers; iter++) {
//			cout << "createGroup iter " << iter << endl;

			int bestUserID;
			float bestSimilarity = - numeric_limits<float>::infinity();
			for (int userID : ur.userIDs) {
				if (group.find(userID) != group.end()) continue; // skip users already selected

				for (int memberID : group ) {
					float sim = computeCosSimilarity(memberID, userID);
					if (sim > bestSimilarity) {
						bestSimilarity = sim;
						bestUserID = userID;
					}
				}
			}
			group.insert(bestUserID);

		}
	}

	if (groupType == 2) { // dissimilar
		group = ur.selectRandomUsers(1);

		for (int iter=1; iter<numUsers; iter++) {
//			cout << "createGroup iter " << iter << endl;

			int worstUserID;
			float worstSimilarity =  numeric_limits<float>::infinity();
			for (int userID : ur.userIDs) {
				if (group.find(userID) != group.end()) continue; // skip users already selected

				for (int memberID : group ) {
					float sim = computeCosSimilarity(memberID, userID);
					if (sim < worstSimilarity) {
						worstSimilarity = sim;
						worstUserID = userID;
					}
				}
			}
			group.insert(worstUserID);

		}
	}


	vector<int> groupVec(group.begin(), group.end());
	setUsers(groupVec);
	computeGroupSimilarityOnSelectedItems();
	return;
}



bool ParetoOptimal::dominates(vector<float> left, vector<float> right) {
	assert (left.size() == right.size());

//	cout << "left: " << vecToStr(left) << endl;
//	cout << "right: " << vecToStr(right) << endl;


	bool strict = false;
	for (int i=0; i<left.size(); i++) {
		if (left[i] < right[i]) {
			return false;
		}
		else if (left[i] > right[i]) {
			strict = true;
		}
	}
	return strict;
}


void ParetoOptimal::findParetoOptimal() {
	for (auto itemRating : sharedRatings) {
		int item = itemRating.first;
		vector<float> ratings = itemRating.second;

		bool isPO = true;
		for (auto otherItemRating : sharedRatings) {
			if (otherItemRating.first == item){
				continue;
			}
			vector<float> otherRatings = otherItemRating.second;
			if (dominates(otherRatings, ratings)) {
				isPO = false;
				break;
			}
		}
		if (isPO) {
//			cout << "item " << item << " is pareto optimal " << vecToStr(sharedRatings[item]) << endl;
			poItemIDs.push_back(item);
			poItems.push_back(Item(item,sharedRatings[item]));
		}
	}
	cout << poItemIDs.size() << " pareto optimal items" << endl;

}


void ParetoOptimal::findKSkyband(int K) {
	for (auto itemRating : sharedRatings) {
		int item = itemRating.first;
		vector<float> ratings = itemRating.second;

		bool isPO = true;
		int countDominants = 0;
		for (auto otherItemRating : sharedRatings) {
			if (otherItemRating.first == item){
				continue;
			}
			vector<float> otherRatings = otherItemRating.second;
			if (dominates(otherRatings, ratings)) {
				countDominants++;
				isPO = false;
			}
		}
		if (countDominants < K) {
//			cout << "item " << item << " is " << (isPO? "" : "not ") << "skyline and is in K-skyband " << vecToStr(sharedRatings[item]) << endl;
			poItemIDs.push_back(item);
			poItems.push_back(Item(item,sharedRatings[item]));
		}
	}
	cout << poItemIDs.size() << " " << K << "-skyband items" << endl;


}


class CompareRankScores
{
public:
	int type;

	CompareRankScores(int type) {
		this->type = type;
	}


	double getScore(const Item& item) {
		double score;
		if (type == AVG) {
			score = 0;
			for (auto val : item.rankScores) {
				score += val;
			}
		}
		else if (type == MED) {
			score = 0;
			multiset<int> rankScores;
			for (auto val : item.rankScores) {
				rankScores.insert(val);
			}

			multiset<int>::iterator iter = rankScores.begin();
			if (rankScores.size() % 2 == 1) {
				advance(iter, rankScores.size()/2);
				score = *iter;
			}
			else {
				advance(iter, rankScores.size()/2 - 1);
				score = *iter;
				iter++;
				score += *iter;
			}

		}
		return score;
	}

	bool operator()(const Item& lhs, const Item& rhs) {

		if (type == CMP) {
			int leftBetter = 0;
			for (int i=0; i<lhs.rankScores.size(); i++) {
				if (lhs.rankScores[i] > rhs.rankScores[i]) {
					leftBetter++;
				}
				else if (lhs.rankScores[i] < rhs.rankScores[i]) {
					leftBetter--;
				}
			}
			return leftBetter > 0;
		}


		double lScore = getScore(lhs);
		double rScore = getScore(rhs);

		if (lScore == rScore) {
			return lhs.id < rhs.id;
		}
		else {
			return lScore > rScore;
		}
	}
};



class CompareRatings
{
public:
	int type;
	vector<float> weights;
	double pen_strength; // strength of penalty for penalty-based

	CompareRatings(int type) {
		this->type = type;
	}


	double getScore(const Item& item) {
		double score;
		if (type == AVG) {
			score = 0;
			for (auto val : item.ratings) {
				score += val;
			}
		}
		else if (type == WEI) {
			score = 0;
			int weightIdx = 0;
			for (auto val : item.ratings) {
				score += weights[weightIdx] * val;
				weightIdx++;
			}
		}
		else if (type == MIN) {
			score = numeric_limits<double>::infinity();
			for (auto val : item.ratings) {
				score = (val < score)? val : score;
			}
		}
		else if (type == MAX) {
			score = - numeric_limits<double>::infinity();
			for (auto val : item.ratings) {
				score = (val > score)? val : score;
			}
		}
		else if (type == PRD) {
			score = 1;
			for (auto val : item.ratings) {
				score *= val;
			}
		}
		else if (type == PEN) {
			double sum = 0;
			double sum_sq = 0;
			for (auto val : item.ratings) {
				sum += val;
				sum_sq += val*val;
			}
			int N = item.ratings.size();
			double var = sum_sq/N - (sum/N)*(sum/N);
			score = sum/N - pen_strength * sqrt(var);

		}
		return score;
	}

	bool operator()(const Item& lhs, const Item& rhs) {
		double lScore = getScore(lhs);
		double rScore = getScore(rhs);

		if (lScore == rScore) {
			return lhs.id < rhs.id;
		}
		else {
			return lScore > rScore;
		}
	}
};



class CompareCounts
{
public:
	bool operator()(const pair<int, int>& lhs, const pair<int, int>& rhs) { // id, number of times selected
		if (lhs.second == rhs.second) {
			return lhs.first < rhs.first;
		}
		else {
			return lhs.second > rhs.second;
		}
	}

};


int ParetoOptimal::getTopItem(int aggType, vector<float> weights) {
	CompareRatings cmp(aggType);
	cmp.weights = weights;

	sort(poItems.begin(), poItems.end(), cmp);

	return poItems[0].id;

}


map<int, int> ParetoOptimal::countTopItemAppearancesOverAllWeights() {

	map<int, int> counts; // itemID, number of times selected

	// initialize
	for (int item : poItemIDs) {
		counts[item] = 0;
	}

	// create many random weights
//	random_device rd;
//	mt19937 mt_rand(rd());
//	if (globalRandomSeed != -1) {
//		mt_rand.seed(globalRandomSeed);
//	}
//
//	uniform_real_distribution<> unif(0, 1);
//	for (int N=0; N<500; N++) {
//
//		vector<float> weights;
//		double norm = 0;
//		for (int i=0; i<users.size(); i++) {
//			float val = unif(mt_rand);
//			norm += val * val;
//			weights.push_back(val);
//		}
//		norm = sqrt(norm);
//
//		for (int i=0; i<weights.size(); i++) {
//			weights[i] /= norm;
//		}
//
//		int bestItemID = getBestItem(WEI, weights);
//		counts[bestItemID]++;
//
//		cout << "W:: " << vecToStr(weights) << endl;
//	}


	int numUsers = users.size();
	int numWeights = pow(5, numUsers);
//	cout << "numWeights: " << numWeights << endl;

	for (int i=1; i<numWeights; i++) {
		vector<float> weights;
		double norm = 0;
		int index = i;
		for (int u=0; u<numUsers; u++) {
			int coord = index % 5;
			index = index / 5;
			float val = ((float) coord)/(5-1);
			norm += val * val;
			weights.push_back(val);
		}
		norm = sqrt(norm);

//		for (int i=0; i<weights.size(); i++) {
//			weights[i] /= norm;
//		}

		int bestItemID = getTopItem(WEI, weights);
		counts[bestItemID]++;

//		cout << "W:: " << vecToStr(weights) << endl;

	}



	cout << "item counts:: ";
	for (auto itemCount : counts) {
		cout << itemCount.first << ":" << itemCount.second << " ";
	}
	cout << endl;

	return counts;

}


vector<int> ParetoOptimal::getTopKItems(int aggType, int K, vector<float> weights) {

	CompareRatings cmp(aggType);
	cmp.weights = weights;

	sort(items.begin(), items.end(), cmp);

//	if (aggType != WEI) {
//		for (Item item : items) {
//			cout << item.id << ":" << cmp.getScore(item) << " ";
//		}
//		cout << endl;
//	}

	vector<int> topK;
	for (int i=0; i<K; i++) {
		topK.push_back( items[i].id );
	}

	if (aggType != WEI) {
		cout << GroupAggTypeNames[aggType] << ": " << vecToStr(topK) << endl;
	}

	return topK;

}


vector<int> ParetoOptimal::getTopKItemsForUserIndx(int K, int userIndx) {
	vector<Item> userItems;

	for (Item item: items){
		vector<float> rating;
		rating.push_back(item.ratings[userIndx]);
		Item userItem(item.id, rating);
		userItems.push_back(userItem);
	}

	CompareRatings cmp(AVG);

	sort(userItems.begin(), userItems.end(), cmp);
	vector<int> topK;
	for (int i=0; i<K; i++) {
		topK.push_back( userItems[i].id );
	}

	cout << "U" << users[userIndx] << ": " << vecToStr(topK) << endl;

	return topK;

}


map<int, int> ParetoOptimal::countTopKAppearancesOverAllWeights(int K) {
	map<int, int> counts; // itemID, number of times selected



	int numUsers = users.size();
	int numWeights = pow(5, numUsers);
	cout << "numWeights: " << numWeights << endl;

	for (int i=1; i<numWeights; i++) {
		vector<float> weights;
		double norm = 0;
		int index = i;
		for (int u=0; u<numUsers; u++) {
			int coord = index % 5;
			index = index / 5;
			float val = ((float) coord)/(5-1);
			norm += val * val;
			weights.push_back(val);
		}
		norm = sqrt(norm);

//		for (int i=0; i<weights.size(); i++) {
//			weights[i] /= norm;
//		}

		vector<int> topK = getTopKItems(WEI, K, weights);
		for (int item : topK) {
			if (counts.find(item) != counts.end()) {
				counts[item]++;
			}
			else {
				counts[item] = 1;
			}
		}

//		cout << "W:: " << vecToStr(weights) << endl;

	}



	cout << "item counts:: ";
	for (auto itemCount : counts) {
		cout << itemCount.first << ":" << itemCount.second << " ";
	}
	cout << endl;


	vector<pair<int, int>> countsVec;
	for (auto itemCount : counts) {
		countsVec.push_back(make_pair(itemCount.first, itemCount.second));
	}
	CompareCounts cc;
	sort(countsVec.begin(), countsVec.end(), cc);


	cout << "CNT: ";
	int num = 0;
	for (auto itemCount : countsVec) {
		cout << itemCount.first << " ";
		num++;
		if (num == K) break;
	}
	cout << endl;

	return counts;
}


/////// top-K probabilities

map<int, float> ParetoOptimal::countTopKProbabilities(int K, int granularity) { // compute the top-K counts (probabilities)
	map<int, float> counts; // itemID, number of times selected

	// initialize counts
	for (auto item : items) {
		counts[item.id] = 0;
	}

	// construct different weights
	int numUsers = users.size();

	if (numUsers > 10) {
		granularity = 2;
	}
	int numWeights = pow(granularity, numUsers);
//	cout << "numWeights: " << numWeights << endl;

	for (int i=1; i<numWeights; i++) {
		vector<float> weights;
//		double norm = 0;
		int index = i;
		for (int u=0; u<numUsers; u++) {
			int coord = index % granularity;
			index = index / granularity;
			float val = ((float) coord)/(granularity-1);
//			norm += val * val;
			weights.push_back(val);
		}
//		norm = sqrt(norm);

//		for (int i=0; i<weights.size(); i++) {
//			weights[i] /= norm;
//		}

		vector<int> topK = getTopKItems(WEI, K, weights);
		for (int item : topK) {
			counts[item] += 1;
		}

	}

	return counts;

}


map<int, float> ParetoOptimal::countsForTopKMostProbableItems(int K, int granularity) { // compute the top-K counts (probabilities)

	map<int, float> counts;

	int nonzero = 0;

	int Z = 3; // for the Z-skyband
	while (nonzero < K) {
		counts = countTopKProbabilities(Z, granularity);
		nonzero = 0;
		for (auto entry : counts) {
			if (abs(entry.second) > 1e-5 ) {
				nonzero++;
			}
		}
//			cout << "nonzero counts " << nonzero << " for K=" << K << endl;
		Z++;
	}
	Z--;

	return counts;

}



vector< map<int, float> > ParetoOptimal::getRandomRankings(map<int, float> counts, int numRankings, int L) { // use the top-L items
	multimap<float, int, std::greater<float>> reverseCounts;

	for (auto element : counts) {
		reverseCounts.insert( make_pair(element.second, element.first) );
	}

	topL.clear();
	vector<float> topL_counts;
	vector<float> aggCounts; // prefix aggregated
	float totalCount = 0;
	for (auto element : reverseCounts) {
		topL.push_back(element.second);
		totalCount += element.first;
		aggCounts.push_back(totalCount);
		topL_counts.push_back(element.first);
//		cout << element.second << " " << element.first << " " << totalCount << endl;
		if (topL.size() == L) break;
	}



	vector< map<int, float> > rankings;

	for (int iter=0; iter<numRankings; iter++) {

		int maxCount = totalCount;

		vector<float> localAggCounts = aggCounts;

		map<int, float> ranking;
		for (int rank=0; rank<L; rank++) {

			int randNum = rand() % maxCount;

			vector<float>::iterator loc = upper_bound(localAggCounts.begin(), localAggCounts.end(), randNum);

			int pos = loc - localAggCounts.begin();

			int item = topL[pos];

			ranking[item] = L - rank;

			vector<float>::iterator it;
			for (it=loc; it!=localAggCounts.end(); it++) {
				*it -=  topL_counts[pos];
			}
			maxCount -= topL_counts[pos];


//			cout << "selected at rank " << rank << " item " << item << " with count " << topL_counts[pos] << "; maxCount=" << maxCount << endl;

		}

		rankings.push_back(ranking);

	}


	return rankings;

}


map<int, float> ParetoOptimal::restrictToTopL(map<int, float> input) {

	map<int, float> output;

	for (auto element : input) {
		if (find(topL.begin(), topL.end(), element.first) != topL.end()) {
			output[element.first] = element.second;
		}

	}

	return output;
}


map<int, float> ParetoOptimal::restrictToN(map<int, float> input, int N) {

	multimap<float, int, std::greater<float>> reverseScores;

	for (auto element : input) {
		reverseScores.insert( make_pair(element.second, element.first) );
	}


	map<int, float> output;


	int cnt = 0;
	for (auto element : reverseScores) {
		if (cnt == N) break;
		output[element.second] = element.first;
//		cout << element.first << " ";
		cnt++;
	}
//	cout << endl;

	return output;
}



map<int, float> ParetoOptimal::restrictToKSkyband(map<int, float> input, int N) {

	multimap<float, int, std::greater<float>> reverseScores;

	for (auto element : input) {
		reverseScores.insert( make_pair(element.second, element.first) );
	}


	map<int, float> output;


	int cnt = 0;
	for (auto element : reverseScores) {
		if (cnt == N) break;
		if ( find(poItemIDs.begin(), poItemIDs.end(), element.second) == poItemIDs.end() ) {
			cout << "gotcha" << endl;
			continue;
		}
		output[element.second] = element.first;
//		cout << element.first << " ";
		cnt++;
	}
//	cout << endl;

	return output;
}



///////////////////////



map<int, float> ParetoOptimal::countTopKOverAllWeights(int K, int type, int granularity) {
	map<int, float> counts; // itemID, number of times selected

	// initialize counts
	for (auto item : items) {
		counts[item.id] = 0;
	}

	// construct different weights
	int numUsers = users.size();
	int numWeights = pow(granularity, numUsers);
//	cout << "numWeights: " << numWeights << endl;

	for (int i=1; i<numWeights; i++) {
		vector<float> weights;
		double norm = 0;
		int index = i;
		for (int u=0; u<numUsers; u++) {
			int coord = index % granularity;
			index = index / granularity;
			float val = ((float) coord)/(granularity-1);
			norm += val * val;
			weights.push_back(val);
		}
		norm = sqrt(norm);

//		for (int i=0; i<weights.size(); i++) {
//			weights[i] /= norm;
//		}

		vector<int> topK = getTopKItems(WEI, K, weights);
		for (int item : topK) {
			counts[item] += 1;
		}

	}

//	for (auto item : items) {
//		counts[item.id] /= numWeights;
//	}
//

	// adjust counts to solve ties
	for (auto& itemCount : counts) {
		int itemID = itemCount.first;
		float score = itemCount.second;

		Item item = getItemByID(items, itemID);
		float avgRankScore = 0.0;
		float minRankScore = items.size();
		float maxRankScore = 0.0;
		float minRating = -1;
		float maxRating = -1;
		for (int i=0; i<numUsers; i++) {
			avgRankScore += item.rankScores[i];
			if (item.rankScores[i] > maxRankScore) {
				maxRankScore = item.rankScores[i];
			}
			if (item.rankScores[i] < minRankScore) {
				minRankScore = item.rankScores[i];
			}
			if (minRating == -1 || item.ratings[i] < minRating) {
				minRating = item.ratings[i];
			}
			if (maxRating == -1 || item.ratings[i] > maxRating) {
				maxRating = item.ratings[i];
			}
		}
		maxRankScore /= items.size();
		minRankScore /= items.size();
		avgRankScore /= (numUsers * items.size());
//		cout << "maxRankScore=" << maxRankScore << " minRankScore=" << minRankScore << " avgRankScore=" << avgRankScore << " minRating=" << minRating << " maxRating=" << maxRating << endl;
		if (type == 0) {
			counts[itemID] += avgRankScore;
		}
		else if (type == 1) {
			counts[itemID] *= avgRankScore;
		}
		else if (type == 2) {
			counts[itemID] *= minRankScore;
		}
		else if (type == 3) {
			counts[itemID] *= minRankScore * avgRankScore;
		}
	}


	return counts;
}



float ParetoOptimal::RecSys17_ComputeVariance(map<int, float> current, int newItemID) {

	vector<float> utilities(users.size(), 0);

	// compute utilities for each user
	for (auto entry : current) {
		Item item = getItemByID(items, entry.first);
		for (int i=0; i<item.ratings.size(); i++) {
			utilities[i] += item.ratings[i];
		}
	}

	// include the utility of newItemID
	Item item = getItemByID(items, newItemID);
	for (int i=0; i<item.ratings.size(); i++) {
		utilities[i] += item.ratings[i];
		utilities[i] /= (current.size() + 1); // divide by number of items
	}


	double s = 0;
	double ss = 0;

	for (float util : utilities) {
		s += util;
		ss += util*util;
	}
	int n = users.size();

	return (1 - (ss/n - s*s/(n*n) ));

}


map<int, float> ParetoOptimal::RecSys17_fair(float lambda) {

	map<int, float> scores;

	int numItems = items.size();

	map<int, float> itemIDsRemaining; // itemID:avgRatingInGroup

	float bestAvgRating = -1;
	int bestItemID;
	/// compute SW social welfare (= group average rating) for each item
	for (auto& item: items) {
		float avgRating = 0;
		for (auto rating : item.ratings) {
			avgRating += rating;
		}
		avgRating /= item.ratings.size();

		if (avgRating > bestAvgRating) {
			bestAvgRating = avgRating;
			bestItemID = item.id;
		}

		itemIDsRemaining[item.id] = avgRating;
	}

	// insert item with best SW; use borda counts for scores
	scores[bestItemID] = numItems;
	itemIDsRemaining.erase(bestItemID);



	for (int rank=1; rank<numItems; rank++) {
		int bestItemID;
		float bestValue = -numeric_limits<float>::infinity();
		for (auto entry : itemIDsRemaining) {
			float sw = entry.second;
			float itemID = entry.first;

			float variance = RecSys17_ComputeVariance(scores, itemID);

			float value = lambda * sw + (1-lambda) * variance;

//			cout << "sw=" << sw << " var=" << variance << " value=" << value << endl;

			if (value > bestValue) {
				bestValue = value;
				bestItemID = itemID;
			}

		}

//		cout << "best has sw " << itemIDsRemaining[bestItemID] << endl;

		scores[bestItemID] = numItems - rank;
		itemIDsRemaining.erase(bestItemID);

	}


	return scores;
}




map<int, float> ParetoOptimal::KAIS_BestPackageGreedy(int numRelevant, map<int, float>& probs) {

	int numItems = items.size();

	map<int, float> results;

	set<int> itemsRemaining;
	for (auto& item: items) {
		itemsRemaining.insert(item.id);
	}


	for (int iter=1; iter<numItems; iter++) {

		int worstItemID;
		assert(numeric_limits<float>::has_infinity == true);
		float bestLogScore = -numeric_limits<float>::infinity();
		for (int excludeItemID : itemsRemaining) {

			// compute score of items in itemsRemaining excluding excludeItemID
			vector<int> selection;
			float selectionLogScore = 0;
			for (int itemID : itemsRemaining) {
				if (itemID == excludeItemID) continue;
				selection.push_back(itemID);
				selectionLogScore += log(probs[itemID]);
			}
			float fairness = KAIS_computeFairness(selection, numRelevant);
			selectionLogScore += log(fairness);

			if (selectionLogScore > bestLogScore) {
				bestLogScore = selectionLogScore;
				worstItemID = excludeItemID;
			}

		}

		results[worstItemID] = iter;
		itemsRemaining.erase(worstItemID);
	}

	int topItemID = *itemsRemaining.begin();
	results[topItemID] = numItems;

//	cout << "itemsRemaining size " << itemsRemaining.size() << endl;
//	cout << "results size " << results.size() << endl;

	return results;
}


map<int, float> ParetoOptimal::KAIS_BestPackage(int numResults, int numRelevant, map<int, float>& probs) {
	/// iterate over all combinations of numResults items; choose k out of n
	int n = items.size();
	int k = numResults;
	vector<int> best;
	float bestScore = 0;
	assert(numeric_limits<float>::has_infinity == true);
	float bestLogScore = -numeric_limits<float>::infinity();
	vector<int> selected; // itemIDs
	vector<int> selector(n);
	fill(selector.begin(), selector.begin() + k, 1);
	do {
		for (int i = 0; i < n; i++) {
			if (selector[i]) {
				selected.push_back(items[i].id); // itemIDs
			}
		}
		// compute score for package
		float score = KAIS_computeFairness(selected, numRelevant);
		double logScore = log(score);
		//		cout << "fairness " << score << endl;
		//		cout << "items: ";
		for (int itemID : selected) {
			//			cout << itemID << " ";
			score *= probs[itemID];
			logScore += log(probs[itemID]);
		}
		//		cout << endl;
		//		cout << "final score " << score << endl;
		//		cout << "final exp(logScore) " << exp(logScore) << endl;
		score = exp(logScore);
		if (logScore > bestLogScore) {
			//			cout << "new best logScore " << logScore << endl;
			best = selected;
			bestScore = score;
		}
		selected.clear();
	} while (prev_permutation(selector.begin(), selector.end()));

	map<int, float> scores; // itemID, final score

	for (Item& item : items) { // set score to 0 for others
		int itemID = item.id;
		scores[itemID] = 0;
	}

	for (int itemID : best) { // set score to probs for the best
		scores[itemID] = probs[itemID];
	}

	return scores;
}

map<int, float> ParetoOptimal::KAIS_fair(int numRelevant) {


	map<int, vector<float> > itemUserProbs;

	vector<float> userTotalRating(users.size(), 0.0);
	for (Item item : items) {
		for (int i=0; i<users.size(); i++) {
			userTotalRating[i] += item.ratings[i];
		}
	}

	for (Item item : items) {
		int itemID = item.id;
		for (int i=0; i<users.size(); i++) {
			itemUserProbs[itemID].push_back( item.ratings[i] /  userTotalRating[i] );
//			cout << "item " << itemID << " user " << i << " itemUserProb " << item.ratings[i] /  userTotalRating[i] << endl;
		}
	}


	vector<float> weights; // user weights = relative number of ratings
	int sumWeights = 0;
	for (int i=0; i<users.size(); i++) {
		int userID = users[i];
		weights.push_back( ur.ratings[userID].size() );
		sumWeights += ur.ratings[userID].size();
	}

	for (int i=0; i<users.size(); i++) {
		int userID = users[i];
		weights[i] /= sumWeights;
//		cout << "user " << userID << " weight " << weights[i] << endl;
	}


	map<int, float> probs;  // itemID, prob for the group

	for (Item item : items) {
		int itemID = item.id;
		probs[itemID] = 0;
		for (int i=0; i<users.size(); i++) {
			probs[itemID] += weights[i] * itemUserProbs[itemID][i];
		}
//		cout << "item " << itemID << " prob " << probs[itemID] << endl;
	}


	map<int, float> scores;

//	scores = KAIS_BestPackage(items.size()-2, numRelevant, probs); // NEVER RUN
	scores = KAIS_BestPackageGreedy(numRelevant, probs);


return scores;

}


float ParetoOptimal::KAIS_computeFairness(vector<int> itemIDs, int numRelevant) {

	vector<bool> isFairToUser;
	for (int i=0; i<users.size(); i++) {
		isFairToUser.push_back(false);
	}

	for (int itemID : itemIDs) {

		Item item = getItemByID(items, itemID);

		for (int i=0; i<users.size(); i++) {
			int rankScore =  item.rankScores[i];
			if ( rankScore > items.size() - numRelevant) {
				isFairToUser[i] = true;  // if at least one item for user i ranks among the top numRelevant items, then it's fair to user i
			}
		}

	}

	int fairnessCount = 0;
	for (int i=0; i<users.size(); i++) {
		if ( isFairToUser[i] ) {
			fairnessCount++;
		}
	}

	return ((float) fairnessCount) / users.size();

}



map<int, float> ParetoOptimal::WWW_fair(float proportion, int m, int type) {
	int numItems = items.size();
	int numUsers = users.size();

	int numRelevant = proportion * numItems; // for proportionality
	int position = proportion * numUsers; // for envy-freeness

//	cout << "numRelevant " << numRelevant << endl;


	map<int, float> scores; // ranking

	// find users satisfied by each item
	map<int, vector<bool> > sat;

	for (Item item : items) {
		int itemID = item.id;
		sat[itemID] = vector<bool>(numUsers);
		if (type == 0) { // proportionality
			for (int i=0; i<users.size(); i++) {
				if (item.rankScores[i] > items.size() - numRelevant) {
					sat[itemID][i] = true;
				}
			}
		}
		else if (type == 1) { // envy-freeness
			vector<float> temp = item.ratings;
			sort(temp.begin(), temp.end(), greater<float>());
			float thresh = temp[position];
			for (int i=0; i<numUsers; i++) {
				if (item.ratings[i] >= thresh) {
					sat[itemID][i] = true;
				}
			}
		}
	}

	set<int> selectedItemIDs;
	vector<bool> groupSat(numUsers, false);
	for (int iter=0; iter<numItems; iter++) {

		int bestItemID;
		float bestUtility = -1;
		for (Item item : items) {
			int itemID = item.id;
			if (selectedItemIDs.find(itemID) != selectedItemIDs.end()) {
				continue; // skip already inserted items
			}
			int utility = 0;
			for (int i=0; i<numUsers; i++) {
				if (!groupSat[i] && sat[itemID][i]) {
					utility++;
				}
			}
			if (utility > bestUtility) {
				bestUtility = utility;
				bestItemID = itemID;
			}
		}

		// next best item found
//		cout << "bestItemID " << bestItemID << " bestUtility " << bestUtility << endl;
		selectedItemIDs.insert(bestItemID);
		scores[bestItemID] = numItems - iter;
		for (int i=0; i<numUsers; i++) {
			groupSat[i] = groupSat[i] || sat[bestItemID][i];
		}


	}

	return scores;

}




void ParetoOptimal::computeRankScores() {

	for (int i=0; i<users.size(); i++) {
		multimap<float, int> ratingItems; // rating:itemID
		for (Item item : items) {
			ratingItems.insert(make_pair(item.ratings[i], item.id));
		}

		map<int, int> itemRankScore; // itemID:rankScore
		int maxRankScore = 1;
		for (auto ratingItem : ratingItems) {
			itemRankScore[ratingItem.second] = maxRankScore++;
		}

		for (Item &item : items) {
			item.rankScores[i] = itemRankScore[item.id];
		}
	}
}



vector<float> ParetoOptimal::getRandomWeights() {
    random_device rd;
    mt19937 g(rd());

    int numUsers = users.size();

    uniform_real_distribution<> dis(1, 10);

    float fracActiveUsers = 1; // 0.5
    int numActiveUsers = fracActiveUsers * numUsers;

    vector<float> weights;

	for (int i=0; i<numUsers; i++) {
		vector<int> selector(numUsers, 0);
		fill(selector.begin(), selector.begin() + numActiveUsers, 1);

		shuffle(selector.begin(), selector.end(), g);
		if (selector[i] == 0) {
			weights.push_back(0);
		}
		else {
			weights.push_back(dis(g));
		}
	}


	return weights;
}


map<int, float> ParetoOptimal::aggregateRatings(int aggType, vector<float> weights, bool onlyPareto) {
	CompareRatings cmp(aggType);
	cmp.weights = weights;

	cmp.pen_strength = 0.05;

	sort(items.begin(), items.end(), cmp);

	map<int, float> aggregate;

	for (Item item : items) {
		int itemID = item.id;
		float score;
		if (onlyPareto && find(poItemIDs.begin(), poItemIDs.end(), itemID) != poItemIDs.end()) {
			score = 0;
		}
		else {
			score = cmp.getScore(item);
		}
		aggregate[itemID] = score;
	}

	return aggregate;
}


map<int, float> ParetoOptimal::localKemenize(map<int, float> initial, vector<Item> inputItems){

	vector<int> orderedItemIDs;
	multimap<float, int, std::greater<float>> flipInitial;
	for (auto entry : initial) {
		flipInitial.insert( make_pair(entry.second, entry.first) );
	}

	for (auto entry : flipInitial) {
		orderedItemIDs.push_back(entry.second);
	}

	int numItems = orderedItemIDs.size();

	// insertion sort-like
	CompareRankScores cmp(CMP);
	for (int n=1; n<numItems; n++) {

		for (int pos=n; pos>0; pos--) {
			Item lhs = getItemByID(inputItems, orderedItemIDs[pos]);
			Item rhs = getItemByID(inputItems, orderedItemIDs[pos-1]);
			bool swap = cmp.operator ()(lhs, rhs);

			if (swap) {
				int tmp = orderedItemIDs[pos-1];
				orderedItemIDs[pos-1] = orderedItemIDs[pos];
				orderedItemIDs[pos] = tmp;
			}

		}


	}

	map<int, float> kem;
	int rankScore = numItems;
	for (auto entry : orderedItemIDs) {
		kem[entry] = rankScore;
		rankScore--;
	}

	return kem;

}


map<int, float> ParetoOptimal::aggregateRanksOfInput(vector<Item> inputItems, int aggType) {
	CompareRankScores cmp(aggType);

	sort(inputItems.begin(), inputItems.end(), cmp);


	map<int, float> aggregate;

	for (Item item : inputItems) {
		int itemID = item.id;
		float score = cmp.getScore(item);
		aggregate[itemID] = score;
	}

	return aggregate;

}



map<int, float> ParetoOptimal::aggregateRanks(int aggType) {
	CompareRankScores cmp(aggType);

	sort(items.begin(), items.end(), cmp);

	map<int, float> aggregate;

	for (Item item : items) {
		int itemID = item.id;
		float score = cmp.getScore(item);
		aggregate[itemID] = score;
	}

	return aggregate;
}



map<int, float> ParetoOptimal::memberRatings(int userIndx) {
	map<int, float> memberRatings;

	for (Item item : items) {
		int itemID = item.id;
		float score = item.ratings[userIndx];
//		float score = item.rankScores[userIndx]; /// BORDA rank scores instead of ratings
		memberRatings[itemID] = score;
	}

	return memberRatings;
}


string ParetoOptimal::getRankingString(map<int, float> rank, string rankName) {
	ostringstream sout;
	multimap<float, int, greater<float> > scoreItems;
	for (auto itemScore : rank) {
		scoreItems.insert(make_pair(itemScore.second, itemScore.first));
	}

	sout << rankName << ": ";
	for (auto scoreItem : scoreItems) {
		sout << scoreItem.second << " (" << scoreItem.first << ") ";
	}
	sout << endl;
	return sout.str();
}








