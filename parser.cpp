#include "parser.h"

namespace {
	const int START_INSTRUCTIONS_COUNT = 4;
	string start_instructions[] = {"create table", "insert into", "project", "select", "rename"};
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
	bool t1_complex = false, t2_complex = false;
	string table1 = t.next_token();
	if (table1 == "(") {
		table1 = "";
		t1_complex = true;
		string token;
		while (true) {
			token = t.next_token();
			if (token == "X") {
				break;
			}
			table1 += token + " ";
		}
	} else {
		t.next_token();
	}
	string table2 = t.next_token();
	if (table2 == "(") {
		t2_complex = true;
		table2 = rest_of_query(t);
	}
	if (t1_complex && t2_complex) {
		return parse(table1.substr(0, table1.length() - 1)).cross(
			   parse(table2.substr(0, table2.length() - 1)));
	} else if (t1_complex) {
		if (ENV.find(table2) == ENV.end()) {
			throw SYNTAX_ERROR("cross(X)", "Table \'" + table2 + "\' doesn't exist");
		}
		return parse(table1.substr(0, table1.length() - 1)).cross(ENV[table2]);
	} else if (t2_complex) {
		if (ENV.find(table1) == ENV.end()) {
			throw SYNTAX_ERROR("cross(X)", "Table \'" + table1 + "\' doesn't exist");
		}
		return parse(table2.substr(0, table2.length() - 1)).cross(ENV[table1]);
	} else {
		if (ENV.find(table1) == ENV.end()) {
			throw SYNTAX_ERROR("cross(X)", "Table \'" + table1 + "\' doesn't exist");
		}
		if (ENV.find(table2) == ENV.end()) {
			throw SYNTAX_ERROR("cross(X)", "Table \'" + table2 + "\' doesn't exist");
		}
		return ENV[table1].cross(ENV[table2]);
	}
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
	if (table_name == "(") {
		string rest = rest_of_query(t);
		Table t = parse(rest.substr(0, rest.length() - 1));
		return t.project(attrs);
	}
	if (ENV.find(table_name) == ENV.end()) {
		throw SYNTAX_ERROR("project", "Table \'" + table_name + "\'' doesn't exist");
	}
	return ENV[table_name].project(attrs);
}

Table select_table(const string &query) {
	Tokenizer t(query);
	t.next_token();
	int b_ctr = 0;
	string token;
	string select_condition = "";
	do {
		token = t.next_token();
		if (token == "(")
			b_ctr++;
		else if (token == ")")
			b_ctr--;
		select_condition += token + " ";
	} while (b_ctr != 0 && !t.eof());
	if (b_ctr != 0)
		throw SYNTAX_ERROR("select", "Incorrect condition syntax");
	try {
		string table_name = t.next_token();
		Predicate *p = create_predicate(select_condition);
		if (table_name == "(") {
			string rest = rest_of_query(t);
			Table t = parse(rest.substr(0, rest.length() - 1));
			return t.select(p);
		}
		if (ENV.find(table_name) == ENV.end()) {
			throw SYNTAX_ERROR("select", "Table \'" + table_name + "\'' doesn't exist");
		}
		return ENV[table_name].select(p);
	} catch (TABLE_ERROR te) {
		throw SYNTAX_ERROR("select", te.msg);
	}
}

Table rename_table(string query) {
	Tokenizer t(query);
	vector<string> attrs;
	t.next_token();
	string new_name = t.next_token(), token;
	if (new_name == "(") {
		new_name = "";
	}
	if (t.next_token() == "(" || new_name == "") {
		while (true) {
			token = t.next_token();
			if (token == ")") {
				break;
			}
			attrs.push_back(token);
		}
	}
	string table_name = t.next_token();
	if (table_name == "(") {
		string rest = rest_of_query(t);
		Table t = parse(rest.substr(0, rest.length() - 1));
		t.rename(new_name, attrs);
		return t;
	} else {
		if (ENV.find(table_name) == ENV.end()) {
			throw(SYNTAX_ERROR("rename", "No such table \'" + table_name + "\' exists"));
		}
		ENV[table_name].rename(new_name, attrs);
		Table t = ENV[table_name];
		ENV.erase(table_name);
		ENV.insert(pair<string, Table>(new_name, t));
	}
}

Table parse(string query) {
	boost::trim(query);
	if (query.length() > 0 && query[0] == '(' && query[query.length() - 1] == ')') {
		return parse(query.substr(1, query.length() - 2));
	}
	for (int i = 0; i < START_INSTRUCTIONS_COUNT; i++) {
		if (query.find(start_instructions[i]) == 0) {
			switch (i) {
				case 0:
					return create_table(query);
				case 1:
					return insert_into(query);
				case 2:
					return project_table(query);
				case 3:
					return select_table(query);
				case 4:
					return rename_table(query);
			}
		}
	}
	Tokenizer t(query);
	string token = t.next_token();
	int b_ctr = 0;
	if (token == "(") {
		b_ctr++;
	}
	while (b_ctr != 0) {
		token = t.next_token();
		if (token == ")")
			b_ctr--;
		if (token == "(")
			b_ctr++;
		if (t.eof()) {
			throw SYNTAX_ERROR("-", "No meaningful statement was found in the query");
		}
	}
	string op = t.next_token();
	for (int i = 0; i < MIDDLE_INSTRUCTIONS_COUNT; i++) {
		if (op == middle_instructions[i]) {
			switch (i) {
				case 0:
					return cross_table(query);
			}
		}
	}
	throw SYNTAX_ERROR("-", "No meaningful statement was found in the query");
}

string rest_of_query(Tokenizer &t) {
	string rest = "";
	string token;
	while (!t.eof()) {
		token = t.next_token();
		if (t.eof()) {
			rest += token;
			return rest;
		}
		rest += token + " ";
	}
	return rest;
}
