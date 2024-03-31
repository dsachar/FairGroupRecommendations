/*
 * GP_Residual.hpp
 *
 *  Created on: Mar 31, 2015
 *      Author: dimitris
 */

#ifndef SRC_GP_RESIDUAL_HPP_
#define SRC_GP_RESIDUAL_HPP_


#include "GroupPredictor.hpp"


using namespace std;


class GP_Residual : public GroupPredictor {
public:

	GP_Residual(ExpSetting& expSet) : GroupPredictor(expSet) {};


	void predictRatings(string ratingsFile, string groupsFile, string groupRatingsFile);
	string const methodName() { return "RESIDUAL"; }

};


#endif /* SRC_GP_RESIDUAL_HPP_ */
