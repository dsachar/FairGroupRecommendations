/*
 * Item.hpp
 *
 *  Created on: Oct 6, 2017
 *      Author: dimitris
 */

#ifndef SRC_ITEM_HPP_
#define SRC_ITEM_HPP_

#include <vector>
#include <sstream>
using namespace std;


class Item {
public:
	int id;
	vector<float> ratings;
	vector<int> rankScores;

	Item(int id, vector<float> ratings) {
		this->id = id;
		this->ratings = ratings;
		this->rankScores.resize(ratings.size());
	}

	string toString();
};


Item& getItemByID(vector<Item>& items, int itemID);


#endif /* SRC_ITEM_HPP_ */
