#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include "tokenizer.h"
#include <string>
#include "table.h"

using namespace std;

extern map<string, Table> ENV;

Table parse(const string &query);
Table create_query(const string &query);
Table insert_into(const string &query);
Table cross_table(const string &query);

class SYNTAX_ERROR {
public:
	string stmt;
	string msg;
	inline SYNTAX_ERROR(string a_stmt, string a_msg) {
		stmt = a_stmt;
		msg = a_msg;
	}
};

#endif
