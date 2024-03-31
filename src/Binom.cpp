/*
 * Binom.cpp
 *
 *  Created on: Mar 20, 2015
 *      Author: dimitris
 */

#ifndef BINOM_CPP_
#define BINOM_CPP_

#include "Binom.hpp"
#include <cassert>
#include <boost/math/special_functions/binomial.hpp>


uint32_t binom(int n, int k) {
//  cout << "n,k " << n << " " << k << endl;
  assert(0 <= k);
  assert(0 <= n);
  if (k > n) return 0;
  if (n == k) return 1;
  if (k == 0) return 1;
  return static_cast<uint32_t>(boost::math::binomial_coefficient<double>(n, k));
//  return binom(n-1, k-1) + binom(n-1, k);
}

uint32_t encode(const std::set<int>& choices) {
  std::set<int, std::greater<int>> choices_sorted(
      choices.begin(), choices.end());
  int k = choices.size();

  uint32_t result = 0;
  for (int choice : choices_sorted) {
    result += binom(choice, k--);
  }
  return result;
}

std::set<int> decode(uint32_t N, int k) {
  int choice = k - 1;
  while (binom(choice, k) < N) {
    choice++;
  }

  std::set<int> result;
  for (; choice >= 0; choice--) {
    if (binom(choice, k) <= N) {
      N -= binom(choice, k--);
      result.insert(choice);
    }
  }
  return result;
}


#endif /* BINOM_CPP_ */
