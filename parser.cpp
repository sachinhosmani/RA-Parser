#include "parser.h"

namespace {
	const int START_INSTRUCTIONS_COUNT = 4;
	string start_instructions[] = {"create table", "insert into", "project", "select"};
	const int MIDDLE_INSTRUCTIONS_COUNT = 1;
	string middle_instructions[] = {"X"};
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
			token = t.next_token();
			values.push_back(token);
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

Table cross_table(const string &query) {
	Tokenizer t(query);
	string table1 = t.next_token();
	t.next_token();
	string table2 = t.next_token();
	if (ENV.find(table1) == ENV.end()) {
		throw SYNTAX_ERROR("cross(X)", "Table \'" + table1 + "\'' doesn't exist");
	}
	if (ENV.find(table2) == ENV.end()) {
		throw SYNTAX_ERROR("cross(X)", "Table \'" + table2 + "\'' doesn't exist");
	}
	return ENV[table1].cross(ENV[table2]);
}

Table project_table(const string &query) {
	Tokenizer t(query);
	t.next_token();
	if (t.next_token() != "(") {
		throw SYNTAX_ERROR("project", "Missing \'(");
	}
	vector<string> attrs;
	string token;
	while (true) {
		token = t.next_token();
		attrs.push_back(token);
		token = t.next_token();
		if (token != ",") {
			if (token != ")") {
				throw SYNTAX_ERROR("project", "expected )");
			} else {
				break;
			}
		}
	}
	string table_name = t.next_token();
	if (ENV.find(table_name) == ENV.end()) {
		throw SYNTAX_ERROR("project", "Table \'" + table_name + "\'' doesn't exist");
	}
	return ENV[table_name].project(attrs);
}

Table parse(const string &query) {
	for (int i = 0; i < START_INSTRUCTIONS_COUNT; i++) {
		if (query.find(start_instructions[i]) == 0) {
			switch (i) {
				case 0:
					return create_table(query);
				case 1:
					return insert_into(query);
				case 2:
					return project_table(query);
			}
		}
	}
	Tokenizer t(query);
	t.next_token();
	string op = t.next_token();
	for (int i = 0; i < MIDDLE_INSTRUCTIONS_COUNT; i++) {
		if (op == middle_instructions[i]) {
			switch (i) {
				case 0:
					return cross_table(query);
			}
		}
	}
}
