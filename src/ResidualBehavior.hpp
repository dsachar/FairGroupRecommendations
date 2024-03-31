/*
 * ResidualBehavior.hpp
 *
 *  Created on: Mar 20, 2015
 *      Author: dimitris
 */

#ifndef SRC_RESIDUALBEHAVIOR_HPP_
#define SRC_RESIDUALBEHAVIOR_HPP_

#include <map>
#include <iostream>


using namespace std;

class UserRatings;
class GroupRatings;
class Groups;

class ResidualBehavior{
public:

	map<int, map<int,float>> userGroupResidualBehavior; // user:group:residualBehavior


	UserRatings& userRatings;
	GroupRatings& groupRatings;
	Groups& groups;


	ResidualBehavior(UserRatings& userRatings, GroupRatings& groupRatings, Groups& groups);

	void calculateResidualBehavior();
	void writeResidualBehavior(string residualBehaviorFile);

};



#endif /* SRC_RESIDUALBEHAVIOR_HPP_ */
