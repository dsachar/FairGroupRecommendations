/*
 * ParetoOptimal.hpp
 *
 *  Created on: 10 Mar 2017
 *      Author: dimitris
 */

#ifndef SRC_PARETOOPTIMAL_HPP_
#define SRC_PARETOOPTIMAL_HPP_


#include <map>
#include <vector>
#include <sstream>
#include <cmath>
#include <limits>
using namespace std;


#include "UserRatings.hpp"
#include "MatrixFactorization.hpp"
//#include "Evaluator.hpp"
#include "Item.hpp"



class ParetoOptimal {
public:
	UserRatings ur;

	MatrixFactorization mf;

	vector<int> users;

	map<int, vector<float>> sharedRatings; // itemID; vector of ratings


	vector<Item> items; // all items used
	set<int> itemIDs; // all itemIDs used

	vector<int> poItemIDs; // pareto optimal itemIDs
	vector<Item> poItems; // pareto optimal items


	vector<int> topL; // itemIDs with the L highest probabilities of being in the top-K
	map<int, float> restrictToTopL(map<int, float> input);

	map<int, float> restrictToN(map<int, float> input, int N); // restrict to N best items

	map<int, float> restrictToKSkyband(map<int, float> input, int N); // restrict to N best items among the k-skyband


	ParetoOptimal();
	ParetoOptimal(UserRatings& userRatings);


	void clearItems();

	void setRandomItems(int num = -1); // -1 means all items; other value means select num random items
	void setCommonRatedItems(); // assumes users are first given

	void setUsers(vector<int> users);


	vector<float> getRandomWeights();

	void runMF();
	void retrieveRatings(); // get actual or predicted ratings

	pair<float, int> computeGroupSimilarity();

	float computeGroupSimilarityOnSelectedItems();
	float computeCosSimilarity(int userA, int userB);
	void computeAllPairsSimilarities();
	void sampleAllPairsSimilarities();

	void createGroupOLD(int numUsers, int groupType);

	void createGroup(int numUsers, int groupType);

	void createGroupNEW(int numUsers, int groupType);


//	void retrieveCommonRatedItems();
//	void retrieveAllItems(); // predict missing ratings
//	void retrieveRandomItemSubset(int num); // subset of size num of all items


	bool dominates(vector<float> left, vector<float> right); // left better (higher) than right
	void findParetoOptimal();
	void findKSkyband(int K);

	int getTopItem(int aggType, vector<float> weights = vector<float>());
	map<int, int> countTopItemAppearancesOverAllWeights();

	vector<int> getTopKItems(int aggType, int K, vector<float> weights = vector<float>());
	map<int, int> countTopKAppearancesOverAllWeights(int K);

	vector<int> getTopKItemsForUserIndx(int K, int userIndx);


	map<int, float> countTopKOverAllWeights(int K, int type = 0, int granularity = 3);
	map<int, float> aggregateRatings(int aggType, vector<float> weights = vector<float>(), bool onlyPareto = false);
	map<int, float> memberRatings(int userIndx);

	map<int, float> KAIS_fair(int numRelevant); // according to KAIS'17 "Recommending Packages with Validity Constraints to Groups of Users"
	float KAIS_computeFairness(vector<int> itemIDs, int numRelevant);
	map<int, float> KAIS_BestPackage(int numResults, int numRelevant, map<int, float>& probs);
	map<int, float> KAIS_BestPackageGreedy(int numRelevant, map<int, float>& probs);


	map<int, float> WWW_fair(float proportion, int m, int type); // according to WWW'17 "Fairness in Package-to-Group Recommendations"


	map<int, float> RecSys17_fair(float lambda); // according to RecSys'17 "Fairness-Aware Group Recommendation with Pareto-Efficiency"
	float RecSys17_ComputeVariance(map<int, float> current, int newItemID);


	map<int, float> countTopKProbabilities(int K, int granularity = 3);
	map<int, float> countsForTopKMostProbableItems(int K, int granularity = 3);
	vector< map<int, float> > getRandomRankings(map<int, float> counts, int numRankings, int L);




	void computeRankScores(); // for rank aggregation, compute the (Borda) score of an item according to its rank; rank 1 has score N, rank N has score 1
	map<int, float> aggregateRanks(int aggType);
	map<int, float> aggregateRanksOfInput(vector<Item> inputItems, int aggType);

	map<int, float> localKemenize(map<int, float> initial, vector<Item> inputItems);


	string getRankingString(map<int, float> rank, string rankName);
};








#endif /* SRC_PARETOOPTIMAL_HPP_ */
