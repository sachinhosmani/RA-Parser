#include "table.h"

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
}

void Table::insert_tuple(const Tuple &t) {
	tuples.push_back(t);
}

void Table::insert_tuple(const vector<string> &values) {
	Tuple t;
	if (values.size() != attr_names.size())
		throw TABLE_ERROR("Incorrect number of values passed to table");
	vector<string>::const_iterator it = values.begin();
	vector<string>::const_iterator it2 = attr_names.begin();
	for (; it != values.end(); it++, it2++) {
		string type = attr_type_map[*it2];
		boost::any val = *it;
		if (type == "int") {
			val = atoi(it->c_str());
		}
		t.insert(pair<string, boost::any>(*it2, val));
	}
	tuples.push_back(t);
}

Table Table::cross(const Table &t) const {
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
	Table crossed(name + " x " + t.name, c_attr_names, c_attr_types);
	vector<Tuple>::const_iterator it3 = tuples.begin();
	vector<Tuple>::const_iterator it4;
	Tuple::const_iterator it5;
	Tuple::const_iterator it6;
	Tuple tmp;
	for (; it3 != tuples.end(); it3++) {
		for (it4 = t.tuples.begin(); it4 != t.tuples.end(); it4++) {
			for (it5 = it3->begin(); it5 != it3->end(); it5++) {
				tmp.insert(pair<string, boost::any>(it5->first + "1", it5->second));
			}
			for (it6 = it4->begin(); it6 != it4->end(); it6++) {
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
	Table projected("p" + name, p_attr_names, p_attr_types);
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
	Table selected("s" + name, attr_names, attr_types);
	vector<Tuple>::iterator it = tuples.begin();
	for (; it != tuples.end(); it++) {
		if (Table::satisfies(p, *it))
			selected.insert_tuple(*it);
	}
	return selected;
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

static bool Table::satisfies(Predicate *p, const Tuple &t) {
	if (!p)
		throw TABLE_ERROR("Invalid predicate passed");
	if (p->op > 0) {
		if (PREDICATE_OP_SYMBOLS[p->op] == "and") {
			return Table::satisfies(p->left, t) && Table::satisfies(p->right, t);
		} else if (PREDICATE_OP_SYMBOLS[p->op] == "or") {
			return Table::satisfies(p->left, t) || Table::satisfies(p->right, t);
		} else {
			throw TABLE_ERROR("Expected either \'and\' or \'or\'");
		}
	} else {
		return Table::satisfies(p->sp, t);
	}
}

static bool Table::satisfies(Simple_Predicate *p, const Tuple &t) {
	if (!p)
		throw TABLE_ERROR("Invalid predicate passed");
	boost::any left = Table::parse_e_tree(p->left, t);
	boost::any right = Table::parse_e_tree(p->right, t);
	try {
		bool res = check_truth(left, right, p->cond);
		return res;
	} catch (TABLE_ERROR t) {
		throw t;
	}
}

static boost::any Table::parse_e_tree(Expression_Tree *e, const Tuple &t) {
	if (!e)
		throw TABLE_ERROR("invalid expression resulted");
	if (!e->left || !e->right) {
		if (is_int_literal(e->data)) {
			int tmp = boost::lexical_cast(e->data);
			return boost::any(tmp);
		} else if (is_string_literal(e->data)) {
			return e->data;
		} else {
			if (t.find(e->data) == t.end())
				throw TABLE_ERROR("No such attribute \'" + e->data + "\' exists");
			else {
				return t[e->data];
			}
		}

	} else {
		boost::any left = Table::parse_e_tree(e->left, t);
		boost::any right = Table::parse_e_tree(e->right, t);
		if (left.type() != right.type())
			throw TABLE_ERROR("Incompatible types for operation \'" + e->op + "\'");
		if (left.type() == typeid(string)) {
			if (e->op == "+") {
				return boost::any((boost::any_cast<string>(left) +
					   boost::any_cast<string>(right)));
			} else {
				throw TABLE_ERROR("Only + supported on strings")
			}
		} else if (left.type() == typeid(int)){
			if (e->op == "+")
				return boost::any((boost::any_cast<int>(left) +
					   boost::any_cast<int>(right)));
			else if (e->op == "-")
				return boost::any((boost::any_cast<int>(left) -
					   boost::any_cast<int>(right)));
			else if (e->op == "*")
				return boost::any((boost::any_cast<int>(left) *
					   boost::any_cast<int>(right)));
			else if (e->op == "/")
				return boost::any((boost::any_cast<int>(left) /
					   boost::any_cast<int>(right)));
			else {
				throw TABLE_ERROR("Unsupported operation \'" + e->op + "\'")
			}
		}
	}
}

static bool check_truth(boost::any a, boost::any b, int cond) {
	if (left.type() != right.type())
		throw TABLE_ERROR("Incompatible types used in condition");
	if (cond < 0)
		throw TABLE_ERROR("Invalid condition passed");
	string cond_sym = COND_OP_SYMBOLS[cond];
	if (left.type() == typeid(string)) {
		if (cond_sym == "==") {
			return boost::any_cast<string>(a) == boost::any_cast<string>(b);
		} else {
			throw TABLE_ERROR("Unsupported condition type \'" + "\'");
		}
	}
	else if (left.type() == typeid(int)) {
		int left = atoi((boost::any_cast<string>(a)).c_str());
		int right = atoi((boost::any_cast<string>(b)).c_str());
		if (COND_OP_SYMBOLS[cond] == "<") {
			return left < right;
		} else if (COND_OP_SYMBOLS[cond] == ">") {
			return left > right;
		} else if (COND_OP_SYMBOLS[cond] == "==") {
			return left == right;
		} else if (COND_OP_SYMBOLS[cond] == "<=") {
			return left <= right;
		} else if (COND_OP_SYMBOLS[cond] == ">=") {
			return left >= right;
		} else {
			throw TABLE_ERROR("Unsupported condition type \'" + "\'");
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