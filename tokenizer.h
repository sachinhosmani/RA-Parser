#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <sstream>
#include <iostream>
#include <boost/algorithm/string.hpp>

using namespace std;

class Tokenizer {
	string input;
	istringstream ss;
public:
	Tokenizer(const string &in);
	string next_token();
	bool eof();
};

class EOF_ERROR {};

#endif
