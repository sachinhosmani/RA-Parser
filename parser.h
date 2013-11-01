#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include "tokenizer.h"
#include <string>
#include "table.h"
#include <boost/algorithm/string.hpp>

using namespace std;

extern map<string, Table> ENV;

Table parse(string query);
Table create_query(const string &query);
Table insert_into(const string &query);
Table cross_table(const string &query);
Table project_table(const string &query);
Table select_table(const string &query);
Table join_table(const string &query);
Table natural_join(const string &query);
Table order_by(const string &query);
Table union_table(const string &query);
Table intersection_table(const string &query);

string rest_of_query(Tokenizer &t);

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
