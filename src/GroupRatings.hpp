/*
 * GroupRatings.hpp
 *
 *  Created on: Mar 20, 2015
 *      Author: dimitris
 */

#ifndef SRC_GROUPRATINGS_HPP_
#define SRC_GROUPRATINGS_HPP_


#include "common.hpp"

#include <map>
#include <iostream>
#include "Groups.hpp"

class Groups;
class UserRatings;


using namespace std;




class GroupRatings {
public:

	int trainPercentage;
	int numItemsRatedPerGroup;
	int groupBehType = WEI;
	string groupsFile;
	bool roundGroupRatings;

	string groupRatingsFile;

	Groups groups;

	int numGroupRatings;

	map<int,map<int,float>> ratings; // group:item:rate

	GroupRatings();

	void readFrom();

	void createSynth(Groups &groups, UserRatings& userRatings, int numItemsRatedPerGroup, int groupBehType, int trainPercentage, bool roundGroupRatings=false, int maxNumGroupRatings=100000);
	void createMultipleSynth(Groups &groups, UserRatings& userRatings, int numItemsRatedPerGroup, int groupBehType, int trainPercentage, bool roundGroupRatings=false, int maxNumGroupRatings=100000);
	void setGroupRatingsFile();


	void createGroupRatingsForCommonItems(UserRatings& userRatings, int maxNumGroupRatings=100000);
	void createGroupRatingsForRandomItems(UserRatings& userRatings, int maxNumGroupRatings=100000);

	void splitAndWriteGroupRatings();
	void splitAndWriteMultipleGroupRatings();
//	void writeGroupRatings();
	void readGroupRatings(string fileName);

};

#endif /* SRC_GROUPRATINGS_HPP_ */
