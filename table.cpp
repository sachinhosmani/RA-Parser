#include "table.h"

static const int BATCH_SIZE = 50;

static void generic_print(string type, boost::any value) {
	if (type == "varchar")
		cout << boost::any_cast<string>(value);
	else if (type == "int")
		cout << boost::any_cast<int>(value);
}

Table::Table() {
	name = "bad_table";
}

Table::Table(const string &a_name, const vector<string> &a_attr_names, const vector<string> &a_attr_types) {
	attr_names = a_attr_names;
	attr_types = a_attr_types;
	vector<string>::const_iterator it = a_attr_names.begin();
	vector<string>::const_iterator it2 = a_attr_types.begin();
	for(; it != a_attr_names.end() && it2 != a_attr_types.end(); it++, it2++) {
		attr_type_map.insert(pair<string, string>(*it, *it2));
	}
	name = a_name;
	eof = false;
	tuples_read = 0;
	file = (name.find("tmp_") == 0) ? "db/tmp/" + name : "db/" + name;
	f_read = new ifstream;
	f_write = new ofstream;
	f_read->open(file.c_str(), fstream::in);
	f_write->open(file.c_str(), fstream::out | fstream::app);
}

Table::Table(const string &a_file, const string &a_md_file) {
	f_read = new ifstream;
	f_write = new ofstream;
	md_read = new ifstream;
	md_write = new ofstream;
	f_read->open(a_file.c_str(), fstream::in);
	f_write->open(a_file.c_str(), fstream::out | fstream::app);
	md_read->open(a_md_file.c_str(), fstream::in);
	md_write->open(a_md_file.c_str(), fstream::out | fstream::app);
	file = a_file;
	md_file = a_md_file;
	eof = false;
	tuples_read = 0;
	string attr_name, attr_type;
	while (!md_read->eof()) {
		if (name == "")
			*md_read >> name;
		*md_read >> attr_name;
		if (attr_name == "")
			break;
		attr_names.push_back(attr_name);
		*md_read >> attr_type;
		if (attr_type == "")
			throw TABLE_ERROR("Corrupt meta data file: \'" + a_md_file + "\'");
		attr_names.push_back(attr_name);
		attr_types.push_back(attr_type);
	}
	vector<string>::iterator it = attr_names.begin();
	vector<string>::iterator it2 = attr_types.begin();
	for(; it != attr_names.end() && it2 != attr_types.end(); it++, it2++) {
		attr_type_map.insert(pair<string, string>(*it, *it2));
	}
}

void Table::reset() {
	eof = false;
	tuples_read = 0;
	tuples.clear();
	f_read->close();
	f_read->open(file.c_str(), fstream::in);
	md_read->close();
	md_read->open(md_file.c_str(), fstream::in);
}

Tuple Table::next_tuple() {
	if (!buffer_empty()) {
		return tuples[tuples_read++];
	}
	tuples.clear();
	tuples_read = 0;
	string line;
	while (tuples_read < BATCH_SIZE) {
		if (f_read->eof()) {
			eof = true;
		}
		*f_read >> line;
		if (line == "") {
			eof = true;
			break;
		}
		tuples.push_back(line_to_tuple(line));
		tuples_read++;
	}
	if (tuples.size() == 0)
		throw TABLE_ERROR("Table exhausted. The object must be reset.");
	return tuples[tuples_read++];
}

void Table::insert_tuple(const Tuple &t) {
	vector<string>::iterator it = attr_names.begin();
	for (; it != attr_names.end(); it++) {
		Tuple::const_iterator it2 = t.find(*it);
		if (attr_type_map[*it] == "varchar") {
			*f_write << boost::any_cast<string>(*it2);
		} else {
			*f_write << boost::any_cast<int>(*it2);
		}
	}
	*f_write << "\n";
}

void Table::insert_tuple(const vector<string> &values) {
	Tuple t;
	if (values.size() != attr_names.size())
		throw TABLE_ERROR("Incorrect number of values passed to table");
	vector<string>::const_iterator it;
	for (it = values.begin(); it != values.end(); it++) {
		*f_write << *it << ";;";
	}
	*f_write << "\n";
}

Table Table::cross(Table t) {
	vector<string> c_attr_names, c_attr_types;
	vector<string>::const_iterator it = attr_names.begin();
	for (; it != attr_names.end(); it++) {
		c_attr_names.push_back(*it + "1");
		string tmp = (attr_type_map.find(*it))->second;
		c_attr_types.push_back(tmp);
	}
	vector<string>::const_iterator it2 = t.attr_names.begin();
	for (; it2 != t.attr_names.end(); it2++) {
		c_attr_names.push_back(*it2 + "2");
		string tmp = (t.attr_type_map.find(*it2))->second;
		c_attr_types.push_back(tmp);
	}
	Table crossed("tmp_" + name + " x " + t.name, c_attr_names, c_attr_types);
	Tuple it3, it4;
	Tuple::const_iterator it5;
	Tuple::const_iterator it6;
	Tuple tmp;
	for (reset(); !end_of_table(); it3 = next_tuple()) {
		for (t.reset(); !t.end_of_table(); it4 = t.next_tuple()) {
			for (it5 = it3.begin(); it5 != it3.end(); it5++) {
				tmp.insert(pair<string, boost::any>(it5->first + "1", it5->second));
			}
			for (it6 = it4.begin(); it6 != it4.end(); it6++) {
				tmp.insert(pair<string, boost::any>(it6->first + "2", it6->second));
			}
			crossed.insert_tuple(tmp);
			tmp.clear();
		}
	}
	return crossed;
}

Table Table::project(const vector<string> &a_attr_names) {
	vector<string> p_attr_names;
	vector<string> p_attr_types;
	vector<string>::const_iterator it = a_attr_names.begin();
	for (; it != a_attr_names.end(); it++) {
		p_attr_names.push_back(*it);
		p_attr_types.push_back(attr_type_map[*it]);
	}
	Table projected("tmp_p" + name, p_attr_names, p_attr_types);
	vector<Tuple>::iterator it2 = tuples.begin();
	Tuple tmp;
	for (; it2 != tuples.end(); it2++) {
		it = a_attr_names.begin();
		for (; it != a_attr_names.end(); it++) {
			tmp.insert(pair<string, boost::any>(*it, (*it2)[*it]));
		}
		projected.insert_tuple(tmp);
		tmp.clear();
	}
	return projected;
}

Table Table::select(Predicate *p) {
	Table selected("tmp_s" + name, attr_names, attr_types);
	Tuple it;
	for (reset(); !end_of_table(); it = next_tuple()) {
		if (Table::satisfies(p, it))
			selected.insert_tuple(it);
	}
	return selected;
}

void Table::rename(const string &a_name, const vector<string> &attrs) {
	if (attrs.size() != attr_names.size())
		throw TABLE_ERROR("Insufficient new attribute names passed");
	if (a_name != "")
		name = a_name;
	if (attrs.size() == 0)
		return;
	map<string, string> new_attr_type_map;
	vector<string>::iterator it = attr_names.begin();
	vector<string>::const_iterator it2 = attrs.begin();
	for (; it != attr_names.end(); it++, it2++) {
		new_attr_type_map[*it2] = attr_type_map[*it];
	}
	attr_names = attrs;
	attr_type_map = new_attr_type_map;
	md_read->close();
	md_write->close();
	remove(md_file.c_str());
	md_write->open(md_file.c_str(), fstream::out | fstream::app);
	*md_write << name << endl;
	it = attr_names.begin();
	for (; it != attr_names.end(); it++) {
		*md_write << *it << " " << attr_type_map[*it] << endl;
	}
	reset();
}

void Table::print() {
	vector<string>::iterator it0 = attr_names.begin();
	for (; it0 != attr_names.end(); it0++) {
		cout << *it0 << "   ";
	}
	cout << endl;
	vector<Tuple>::iterator it1 = tuples.begin();
	for (; it1 != tuples.end(); it1++) {
		it0 = attr_names.begin();
		for (; it0 != attr_names.end(); it0++) {
			generic_print(attr_type_map[*it0], (*it1)[*it0]);
			cout << "   ";
		}
		cout << "\n";
	}
}

bool Table::satisfies(Predicate *p, const Tuple &t) {
	if (!p)
		throw TABLE_ERROR("Invalid predicate passed");
	if (p->op > 0) {
		if (PREDICATE_OP_SYMBOLS[p->op - 1] == "and") {
			return Table::satisfies(p->left, t) && Table::satisfies(p->right, t);
		} else if (PREDICATE_OP_SYMBOLS[p->op - 1] == "or") {
			return Table::satisfies(p->left, t) || Table::satisfies(p->right, t);
		} else {
			throw TABLE_ERROR("Expected either \'and\' or \'or\'");
		}
	} else {
		return Table::satisfies(p->sp, t);
	}
}

bool Table::satisfies(Simple_Predicate *p, const Tuple &t) {
	if (!p)
		throw TABLE_ERROR("Invalid predicate passed");
	cout << "here\n";
	boost::any left = Table::parse_e_tree(p->left, t);
	boost::any right = Table::parse_e_tree(p->right, t);
	try {
		bool res = check_truth(left, right, p->cond);
		return res;
	} catch (TABLE_ERROR t) {
		throw t;
	}
}

boost::any Table::parse_e_tree(Expression_Tree *e, const Tuple &t) {
	if (!e)
		throw TABLE_ERROR("invalid expression resulted");
	if (!e->left || !e->right) {
		if (is_int_literal(e->data)) {
			int tmp = boost::lexical_cast<int>(e->data);
			return boost::any(tmp);
		} else if (is_string_literal(e->data)) {
			string trimmed = (e->data).substr(1, (e->data).length() - 2);
			return boost::any(trimmed);
		} else {
			Tuple::const_iterator it = t.find(e->data);
			if (it == t.end())
				throw TABLE_ERROR("No such attribute \'" + e->data + "\' exists");
			else {
				return it->second;
			}
		}

	} else {
		boost::any left = Table::parse_e_tree(e->left, t);
		boost::any right = Table::parse_e_tree(e->right, t);
		if (left.type() != right.type())
			throw TABLE_ERROR("Incompatible types for operation \'" + e->data + "\'");
		if (left.type() == typeid(string)) {
			if (e->data == "+") {
				return boost::any((boost::any_cast<string>(left) +
					   boost::any_cast<string>(right)));
			} else {
				throw TABLE_ERROR("Only + supported on strings");
			}
		} else if (left.type() == typeid(int)){
			if (e->data == "+")
				return boost::any((boost::any_cast<int>(left) +
					   boost::any_cast<int>(right)));
			else if (e->data == "-")
				return boost::any((boost::any_cast<int>(left) -
					   boost::any_cast<int>(right)));
			else if (e->data == "*")
				return boost::any((boost::any_cast<int>(left) *
					   boost::any_cast<int>(right)));
			else if (e->data == "/")
				return boost::any((boost::any_cast<int>(left) /
					   boost::any_cast<int>(right)));
			else {
				throw TABLE_ERROR("Unsupported operation \'" + e->data + "\'");
			}
		}
	}
}

bool Table::buffer_empty() {
	return tuples.size() == 0 || tuples_read >= BATCH_SIZE;
}

bool Table::end_of_table() {
	return eof && buffer_empty();
}

Tuple Table::line_to_tuple(const string &line) {
	istringstream ss(line);
	string token;
	vector<string> values;
	while (getline(ss, token, ';')) {
		ss >> token;
		values.push_back(token);
	}
	vector<string>::iterator it = values.begin();
	vector<string>::iterator it2 = attr_names.begin();
	Tuple t;
	for (; it != values.end(); it++, it2++) {
		string type = attr_type_map[*it2];
		boost::any val = *it;
		if (type == "int") {
			val = atoi(it->c_str());
		}
		t.insert(pair<string, boost::any>(*it2, val));
	}
	return t;
}

bool check_truth(boost::any a, boost::any b, int cond) {
	cout << "entered" <<  cond << "\n";
	if (a.type() != b.type())
		throw TABLE_ERROR("Incompatible types used in condition");
	if (cond < 0)
		throw TABLE_ERROR("Invalid condition passed");
	string cond_sym = COND_OP_SYMBOLS[cond - 1];
	if (a.type() == typeid(string)) {
		if (cond_sym == "==") {
			return boost::any_cast<string>(a) == boost::any_cast<string>(b);
		} else {
			throw TABLE_ERROR("Unsupported condition type \'" + cond_sym + "\' passed");
		}
	}
	else if (a.type() == typeid(int)) {
		int left = boost::any_cast<int>(a);
		int right = boost::any_cast<int>(b);
		if (cond_sym == "<") {
			return left < right;
		} else if (cond_sym == ">") {
			return left > right;
		} else if (cond_sym == "==") {
			return left == right;
		} else if (cond_sym == "<=") {
			return left <= right;
		} else if (cond_sym == ">=") {
			return left >= right;
		} else {
			throw TABLE_ERROR("Unsupported condition type \'" + cond_sym + "\' passed");
		}
	}
}

static bool is_string_literal(const string &s) {
	return s.length() > 2 && s[0] == '\"' && s[s.length() - 1] == '\"';
}

static bool is_int_literal(const string &s) {
	try {
		int tmp = boost::lexical_cast<int>(s);
		return true;
	} catch (...) {
		return false;
	}
}
