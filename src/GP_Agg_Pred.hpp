/*
 * GP_Agg_Pred.hpp
 *
 *  Created on: Mar 31, 2015
 *      Author: dimitris
 */

#ifndef SRC_GP_AGG_PRED_HPP_
#define SRC_GP_AGG_PRED_HPP_


#include "GroupPredictor.hpp"
#include "common.hpp"

using namespace std;


class GP_Agg_Pred : public GroupPredictor {
public:

	int groupPredType = AVG;

	GP_Agg_Pred(ExpSetting& expSet, int type);

	void setPredictionType(GroupAggType type);
	void setPredictionType(int type);
	void predictRatings(string ratingsFile, string groupsFile, string groupRatingsFile);
	string const methodName();
};



#endif /* SRC_GP_AGG_PRED_HPP_ */
