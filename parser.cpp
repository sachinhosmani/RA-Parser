#include "parser.h"

namespace {
	const int INSTRUCTIONS_COUNT = 2;
	string instructions[] = {"create table", "insert into"};
}

Table create_table(const string &query) {
	string attr_name, attr_type;
	vector<string> attr_names, attr_types;
	try {
		Tokenizer t(query);
		t.next_token();
		t.next_token();
		string table_name = t.next_token();
		string token;
		token = t.next_token();
		if (token != "(")
			throw SYNTAX_ERROR("create table", "expected (");
		while (true) {
			attr_name = token = t.next_token();
			attr_type = token = t.next_token();
			attr_names.push_back(attr_name);
			attr_types.push_back(attr_type);
			token = t.next_token();
			if (token != ",") {
				if (token != ")") {
					throw SYNTAX_ERROR("create table", "expected )");
				} else {
					break;
				}
			}
		}
		return Table(table_name, attr_names, attr_types);
	} catch (EOF_ERROR e) {
		throw SYNTAX_ERROR("create table", "unexpected end of statement");
	} catch (SYNTAX_ERROR e) {
		throw e;
	}
	return Table("bad_table", attr_names, attr_types);
}

Table insert_into(const string &query) {
	string value;
	vector<string> values;
	try {
		Tokenizer t(query);
		t.next_token();
		t.next_token();
		string table_name = t.next_token();
		if (ENV.find(table_name) == ENV.end()) {
			throw SYNTAX_ERROR("insert into", "no table \'" + table_name + "\' exists");
		}
		string token;
		token = t.next_token();
		if (token != "values") {
			throw SYNTAX_ERROR("insert into", "Expected token \'values\'");
		}
		token = t.next_token();
		if (token != "(")
			throw SYNTAX_ERROR("insert into", "expected (");
		while (true) {
			value = token = t.next_token();
			values.push_back(value);
			token = t.next_token();
			if (token != ",") {
				if (token != ")") {
					throw SYNTAX_ERROR("create table", "expected )");
				} else {
					break;
				}
			}
		}
		ENV[table_name].insert_tuple(values);
		return ENV[table_name];
	} catch (EOF_ERROR e) {
		throw SYNTAX_ERROR("create table", "unexpected end of statement");
	} catch (SYNTAX_ERROR e) {
		throw e;
	}
	vector<string> attr_names, attr_types;
	return Table("bad_table", attr_names, attr_types);
}

Table parse(const string &query) {
	for (int i = 0; i < INSTRUCTIONS_COUNT; i++) {
		if (query.find(instructions[i]) == 0) {
			switch (i) {
				case 0:
					return create_table(query);
					break;
				case 1:
					return insert_into(query);
					break;
			}
		}
	}
}
