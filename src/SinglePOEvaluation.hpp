/*
 * SinglePOEvaluation.hpp
 *
 *  Created on: Oct 6, 2017
 *      Author: dimitris
 */

#ifndef SRC_SINGLEPOEVALUATION_HPP_
#define SRC_SINGLEPOEVALUATION_HPP_

#include <vector>
#include <map>

#include "POEvaluation.hpp"

class POEvaluation;

class SinglePOEvaluation { // for a single repetition
public:

	POEvaluation &poe;

	vector<map<int, float> > criteria; // vector of map<int, float> truth criteria
	vector<map<int, float> > methods; // vector of map<int, float> estim methods

	SinglePOEvaluation(POEvaluation &poe);

	void evaluateMethodsOnCriteria();
};


#endif /* SRC_SINGLEPOEVALUATION_HPP_ */
