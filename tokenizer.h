#include <string>
#include <sstream>
#include <iostream>

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
