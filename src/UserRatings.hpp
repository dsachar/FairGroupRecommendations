/*
 * UserRatings.hpp
 *
 *  Created on: Mar 20, 2015
 *      Author: dimitris
 */

#ifndef SRC_USERRATINGS_HPP_
#define SRC_USERRATINGS_HPP_

#include <map>
#include <set>
#include <string>
using namespace std;


class UserRatings {
public:

	string ratingsFile;

	int numUsers;
	int numItems;
	int numUserRatings;
	float maxRating;

	set<int> userIDs;
	set<int> itemIDs;

	map<int,map<int,float>> ratings; // user:item:rate


	UserRatings();

	void readRatings(string ratingsFile);
	void printRatings();

	pair<float, int> computeCosSimilarity(int userA, int userB, set<int> items = set<int>()); // cosine similarity over mean-centered vectors; https://grouplens.org/blog/similarity-functions-for-user-user-collaborative-filtering/

	void computeAllPairsSimilarities();

	set<int> findNeighborhood(int userID, int k);
	set<int> findFarhood(int userID, int k);

	set<int> findSimilar(int seedUser, int k, set<int> items = set<int>());
	set<int> findDissimilar(int seedUser, int k, set<int> items = set<int>());

	set<int> selectRandomUsers(int k);

};


#endif /* SRC_USERRATINGS_HPP_ */
