/*
 * GroupGenerator.hpp
 *
 *  Created on: Mar 19, 2015
 *      Author: dimitris
 */

#ifndef SRC_GROUPS_HPP_
#define SRC_GROUPS_HPP_


#include <iostream>
#include <cassert>
#include <set>
#include <map>
#include <vector>


#include "common.hpp"


using namespace std;




class Groups {
public:

	int numGroups;
	int groupSize;
	int numDistinctUsersInGroups;
	int userBehType = UNF;

	string ratingsFile;
	string groupsFile;

	// data
	set<int> distinctUsers;
	map<int, map<int, float>> groupUserWeights; // group:user:weight

	set<int> groupIDs;

	Groups();


	void readFrom(string baseDir, string fileName);
	void read(string fileName);

	void createSynth(int numUsers, int numGroups, int groupSize, int numDistinctUsersInGroups, int behaviorType, string baseDir);

	void setGroupsFile(string baseDir);

	void createGroups(int numUsers);
	void assignWeights();
	void writeGroups();
	void readGroups();


};



#endif /* SRC_GROUPS_HPP_ */
