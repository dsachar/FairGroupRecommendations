/*
 * GP_InfMatch.hpp
 *
 *  Created on: Apr 17, 2015
 *      Author: dimitris
 */

#ifndef SRC_GP_INFMATCH_HPP_
#define SRC_GP_INFMATCH_HPP_


#include "GroupPredictor.hpp"


using namespace std;


class GP_InfMatch : public GroupPredictor {
public:

	GP_InfMatch(ExpSetting& expSet) : GroupPredictor(expSet) {};


	void predictRatings(string ratingsFile, string groupsFile, string groupRatingsFile);
	string const methodName() { return "INF-MATCH"; }

};


#endif /* SRC_GP_INFMATCH_HPP_ */
