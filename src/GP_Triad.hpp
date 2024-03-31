/*
 * GP_Triad.hpp
 *
 *  Created on: Apr 1, 2015
 *      Author: dimitris
 */

#ifndef SRC_GP_TRIAD_HPP_
#define SRC_GP_TRIAD_HPP_


#include "GroupPredictor.hpp"


using namespace std;


class GP_Triad : public GroupPredictor {
public:

	GP_Triad(ExpSetting& expSet) : GroupPredictor(expSet) {};


	void predictRatings(string ratingsFile, string groupsFile, string groupRatingsFile);
	string const methodName() { return "TRIAD"; }

};



#endif /* SRC_GP_TRIAD_HPP_ */
