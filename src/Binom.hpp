/*
 * Binom.hpp
 *
 *  Created on: Mar 20, 2015
 *      Author: dimitris
 */

#ifndef SRC_BINOM_HPP_
#define SRC_BINOM_HPP_

#include <set>

using namespace std;

uint32_t binom(int n, int k);
uint32_t encode(const std::set<int>& choices);
std::set<int> decode(uint32_t N, int k);


#endif /* SRC_BINOM_HPP_ */
