#include "tokenizer.h"

Tokenizer::Tokenizer(const string &in) {
	input = in;
	boost::trim(input);
	ss.str(input);
}

string Tokenizer::next_token() {
	try {
		char first, last;
		string token = "";
		ss >> first;
		token += first;
		if (first != '(') {
			ss.unget();
			ss >> token;
		}
		last = token[token.length() - 1];
		while (last == ')' || last == ',') {
			if (token.length() > 1) {
				ss.unget();
				token.erase(token.length() - 1);
				last = token[token.length() - 1];
			} else {
				return token;
			}
		}
		return token;
	} catch (EOF_ERROR e) {
		throw EOF_ERROR();
	}
	return string("");
}

bool Tokenizer::eof() {
	return ss.eof();
}
