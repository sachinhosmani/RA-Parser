#include "table.h"

static const int BATCH_SIZE = 50;

static bool check_truth(boost::any a, boost::any b, int cond);
static bool is_string_literal(const string &s);
static bool is_int_literal(const string &s);
static string random_str_gen(int length);
static bool satisfies_natural_join(const Tuple &t1, const Tuple &t2, const vector<string> &common_attrs);
static bool same(const Tuple &t1, const Tuple &t2, const vector<string> &attrs);
static bool same(const Tuple &t1, const Tuple &t2);
static bool same_type_insensitive(const Tuple &t1, const vector<string> &attr_names_1,
								  const Tuple &t2, const vector<string> &attr_names_2);

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
	tuples.clear();
	file = path("db/tmp/" + random_str_gen(8));
	md_file = path(file.string() + "_metadata");
	f_read = new ifstream;
	f_write = new ofstream;
	md_read = new ifstream;
	md_write = new ofstream;
	f_read->open(get_file_path().c_str(), fstream::in);
	f_write->open(get_file_path().c_str(), fstream::out | fstream::app);
	md_read->open(get_md_file_path().c_str(), fstream::in);
	md_write->open(get_md_file_path().c_str(), fstream::out | fstream::app);
	*md_write << name << "\n";
	it = attr_names.begin();
	for (; it != attr_names.end(); it++) {
		*md_write << *it << ";" << attr_type_map[*it] << "\n";
	}
	md_write->close();
}

Table::Table(const path &a_file, const path &a_md_file) {
	f_read = new ifstream;
	f_write = new ofstream;
	md_read = new ifstream;
	md_write = new ofstream;
	file = a_file;
	md_file = a_md_file;
	f_read->open(get_file_path().c_str(), fstream::in);
	f_write->open(get_file_path().c_str(), fstream::out | fstream::app);
	md_read->open(get_md_file_path().c_str(), fstream::in);
	md_write->open(get_md_file_path().c_str(), fstream::out | fstream::app);
	eof = false;
	tuples_read = 0;
	string attr_name, attr_type;
	string line;
	tuples.clear();
	while (true) {
		if (name == "") {
			getline(*md_read, name);
			continue;
		}
		getline(*md_read, line);
		if (line == "" || md_read->eof())
			break;
		istringstream ss(line);
		getline(ss, attr_name, ';');
		getline(ss, attr_type, ';');
		if (attr_name == "" || attr_type == "")
			throw TABLE_ERROR("Corrupt meta data file: \'" + get_md_file_path() + "\'");
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
	f_read->open(get_file_path().c_str(), fstream::in);
	if (md_read) {
		md_read->close();
		md_read->open(get_md_file_path().c_str(), fstream::in);
	}
}

Tuple Table::next_tuple() {
	if (!buffer_empty()) {
		return tuples[tuples_read++];
	}
	tuples.clear();
	tuples_read = 0;
	string line;
	int i = 0;
	while (i < BATCH_SIZE) {
		if (f_read->eof()) {
			eof = true;
			break;
		}
		getline(*f_read, line);
		if (line == "") {
			/*eof = true;
			break;*/
			continue;
		}
		tuples.push_back(line_to_tuple(line));
		i++;
	}
	if (tuples.size() == 0) {
		throw TABLE_ERROR("Table exhausted. Table is empty or the object must be reset.");
	}
	return tuples[tuples_read++];
}

void Table::insert_tuple(const Tuple &t) {
	f_write->close();
	f_write->open(get_file_path().c_str(), fstream::out | fstream::app);
	vector<string>::iterator it = attr_names.begin();
	while (it != attr_names.end()) {
		Tuple::const_iterator it2 = t.find(*it);
		if (attr_type_map[*it] == "varchar") {
			*f_write << boost::any_cast<string>(it2->second);
		} else {
			*f_write << boost::any_cast<int>(it2->second);
		}
		it++;
		if (it != attr_names.end())
			*f_write << ";";
	}
	*f_write << "\n";
	f_write->close();
}

void Table::insert_tuple(const vector<string> &values) {
	Tuple t;
	if (values.size() != attr_names.size())
		throw TABLE_ERROR("Incorrect number of values passed to table");
	vector<string>::const_iterator it = values.begin();
	while (it != values.end()) {
		*f_write << *it;
		if (it != values.end())
			*f_write << ";";
	}
	*f_write << "\n";
}

Table Table::cross(Table t) {
	vector<string> c_attr_names, c_attr_types;
	vector<string>::const_iterator it = attr_names.begin();
	for (; it != attr_names.end(); it++) {
		c_attr_names.push_back(name + "." + *it);
		string tmp = (attr_type_map.find(*it))->second;
		c_attr_types.push_back(tmp);
	}
	vector<string>::const_iterator it2 = t.attr_names.begin();
	for (; it2 != t.attr_names.end(); it2++) {
		c_attr_names.push_back(t.name + "." + *it2);
		string tmp = (t.attr_type_map.find(*it2))->second;
		c_attr_types.push_back(tmp);
	}
	Table crossed("tmp_" + name + "X" + t.name, c_attr_names, c_attr_types);
	Tuple it3, it4;
	Tuple::const_iterator it5;
	Tuple::const_iterator it6;
	Tuple tmp;
	reset();
	while (!end_of_table()) {
		it3 = next_tuple();
		t.reset();
		while (!t.end_of_table()) {
			it4 = t.next_tuple();
			for (it5 = it3.begin(); it5 != it3.end(); it5++) {
				tmp.insert(pair<string, boost::any>(name + "." + it5->first, it5->second));
			}
			for (it6 = it4.begin(); it6 != it4.end(); it6++) {
				tmp.insert(pair<string, boost::any>(t.name + "." + it6->first, it6->second));
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
	Tuple tmp;
	reset();
	while (!end_of_table()) {
		tmp = next_tuple();
		for (it = a_attr_names.begin(); it != a_attr_names.end(); it++) {
			tmp.insert(pair<string, boost::any>(*it, tmp[*it]));
		}
		projected.insert_tuple(tmp);
		tmp.clear();
	}
	return projected;
}

Table Table::select(Predicate *p) {
	Table selected("tmp_s" + name, attr_names, attr_types);
	Tuple it;
	reset();
	while (!end_of_table()) {
		it = next_tuple();
		if (Table::satisfies(p, it)) {
			selected.insert_tuple(it);
		}
	}
	return selected;
}

void Table::rename(const string &a_name, const vector<string> &attrs) {
	if (attrs.size() > 0 && attrs.size() != attr_names.size())
		throw TABLE_ERROR("Insufficient new attribute names passed");
	if (a_name != "")
		name = a_name;
	if (attrs.size() > 0) {
		map<string, string> new_attr_type_map;
		vector<string>::iterator it = attr_names.begin();
		vector<string>::const_iterator it2 = attrs.begin();
		for (; it != attr_names.end(); it++, it2++) {
			new_attr_type_map[*it2] = attr_type_map[*it];
		}
		attr_names = attrs;
		attr_type_map = new_attr_type_map;
	}
	md_read->close();
	md_write->close();
	remove(md_file.c_str());
	md_write->open(get_md_file_path().c_str(), fstream::out | fstream::app);
	*md_write << name << endl;
	vector<string>::iterator it = attr_names.begin();
	for (; it != attr_names.end(); it++) {
		*md_write << *it << ";" << attr_type_map[*it] << endl;
	}
	md_write->close();
	reset();
}

Table Table::theta_join(Table t, Predicate *p) {
	vector<string> c_attr_names, c_attr_types;
	vector<string>::const_iterator it = attr_names.begin();
	for (; it != attr_names.end(); it++) {
		c_attr_names.push_back(name + "." + *it);
		string tmp = (attr_type_map.find(*it))->second;
		c_attr_types.push_back(tmp);
	}
	vector<string>::const_iterator it2 = t.attr_names.begin();
	for (; it2 != t.attr_names.end(); it2++) {
		c_attr_names.push_back(t.name + "." + *it2);
		string tmp = (t.attr_type_map.find(*it2))->second;
		c_attr_types.push_back(tmp);
	}
	Table joined("tmp_" + name + "join" + t.name, c_attr_names, c_attr_types);
	Tuple it3, it4;
	Tuple::const_iterator it5;
	Tuple::const_iterator it6;
	Tuple tmp;
	bool non_existent_attr = false;
	reset();
	while (!end_of_table()) {
		it3 = next_tuple();
		t.reset();
		while (!t.end_of_table()) {
			it4 = t.next_tuple();
			for (it5 = it3.begin(); it5 != it3.end(); it5++) {
				tmp.insert(pair<string, boost::any>(name + "." + it5->first, it5->second));
			}
			for (it6 = it4.begin(); it6 != it4.end(); it6++) {
				tmp.insert(pair<string, boost::any>(t.name + "." + it6->first, it6->second));
			}
			try {
				if (Table::satisfies(p, tmp)) {
					joined.insert_tuple(tmp);
				}
			}
			catch (TABLE_ERROR e) {
				non_existent_attr = true;
			}
			tmp.clear();
		}
	}
	if (non_existent_attr)
		throw TABLE_ERROR("Non-existent attributes used in the table");
	return joined;
}

Table Table::natural_join(Table t) {
	vector<string>::iterator it = attr_names.begin();
	vector<string> new_attr_names, new_attr_types, common_attrs;
	map<string, bool> added;
	for (; it != attr_names.end(); it++) {
		added.insert(pair<string, bool>(*it, true));
		new_attr_names.push_back(*it);
		new_attr_types.push_back(attr_type_map[*it]);
	}
	for (it = t.attr_names.begin(); it != t.attr_names.end(); it++) {
		if (added.find(*it) != added.end())
			continue;
		added.insert(pair<string, bool>(*it, true));
		new_attr_names.push_back(*it);
		new_attr_types.push_back(t.attr_type_map[*it]);
	}
	for (it = new_attr_names.begin(); it != new_attr_names.end(); it++) {
		if (attr_type_map.find(*it) != attr_type_map.end() &&
			t.attr_type_map.find(*it) != t.attr_type_map.end())
			common_attrs.push_back(*it);
	}
	Table joined(name + "njoin" + t.name, new_attr_names, new_attr_types);
	Tuple t1, t2;
	reset();
	while (!end_of_table()) {
		t1 = next_tuple();
		t.reset();
		Tuple tmp;
		while (!t.end_of_table()) {
			t2 = t.next_tuple();
			if (satisfies_natural_join(t1, t2, common_attrs)) {
				Tuple::const_iterator it2 = t1.begin();
				for (; it2 != t1.end(); it2++) {
					tmp.insert(pair<string, boost::any>(it2->first, it2->second));
				}
				it2 = t2.begin();
				for (; it2 != t2.end(); it2++) {
					tmp.insert(pair<string, boost::any>(it2->first, it2->second));
				}
				joined.insert_tuple(tmp);
				tmp.clear();
			}
		}
	}
	return joined;
}

Table Table::aggregate(const vector<string> &a_group_attrs, const vector<string> &funcs, const vector<string> &attrs) {
	vector<string> group_attrs = a_group_attrs;
	string sort_cmd = "sort -t \";\" ";
	vector<string> old_gp_attrs = group_attrs;
	vector<string> group_attr_types;
	map<string, string> func_attr_map;
	vector<string>::const_iterator it = funcs.begin();
	vector<string>::const_iterator it2 = attrs.begin();
	for (; it != funcs.end(); it++, it2++) {
		func_attr_map.insert(pair<string, string>(*it, *it2));
	}
	vector<int> col_numbers;
	for (it = group_attrs.begin(); it != group_attrs.end(); it++) {
		int ctr = 0;
		group_attr_types.push_back(attr_type_map[*it]);
		for (it2 = attr_names.begin(); it2 != attr_names.end(); it2++) {
			ctr++;
			if (*it2 == *it) {
				col_numbers.push_back(ctr);
			}
		}
	}
	it = funcs.begin();
	it2 = attrs.begin();
	for (; it != funcs.end(); it++, it2++) {
		string a = *it;
		string b = *it2;
		group_attrs.push_back(a + b);
		group_attr_types.push_back("int");
	}
	vector<int>::iterator it3 = col_numbers.begin();
	for (; it3 != col_numbers.end(); it3++) {
		sort_cmd += "-k " + boost::lexical_cast<string>(*it3) + "," + boost::lexical_cast<string>(*it3) + " ";
	}
	sort_cmd += get_file_path();
	sort_cmd += " -o " + get_file_path();
	system(sort_cmd.c_str());
	Table aggregated(name + "aggregated", group_attrs, group_attr_types);
	Tuple t, last_t;
	reset();
	int count = 0, sum = 0;
	bool first = true;
	while (!end_of_table()) {
		t = next_tuple();
		if ((!first && !same(last_t, t, old_gp_attrs)) || end_of_table()) {
			Tuple tmp;
			map<string, string>::iterator it5 = func_attr_map.begin();
			for (; it5 != func_attr_map.end(); it5++) {
				if (it5->first == "sum") {
					tmp[it5->first+it5->second] = boost::any(sum);
				}
				if (it5->first == "avg") {
					tmp[it5->first+it5->second] = boost::any(sum/count);
				}
				if (it5->first == "count") {
					tmp[it5->first+it5->second] = boost::any(count);
				}
			}
			for (it = old_gp_attrs.begin(); it != old_gp_attrs.end(); it++) {
				tmp[*it] = last_t[*it];
			}
			aggregated.insert_tuple(tmp);
			sum = 0; count = 0;
		}
		first = false;
		last_t = t;
		count++;
		map<string, string>::iterator it5 = func_attr_map.begin();
		for (; it5 != func_attr_map.end(); it5++) {
			if (it5->first == "sum") {
				sum += boost::any_cast<int>(t[it5->second]);
			}
		}
	}
	return aggregated;
}

Table Table::order_by(const vector<string> &attrs) {
	string sort_cmd = "sort -t \";\" ";
	vector<string>::const_iterator it, it2;
	vector<int> col_numbers;
	for (it = attrs.begin(); it != attrs.end(); it++) {
		int ctr = 0;
		for (it2 = attr_names.begin(); it2 != attr_names.end(); it2++) {
			ctr++;
			if (*it2 == *it) {
				col_numbers.push_back(ctr);
			}
		}
	}
	vector<int>::iterator it3 = col_numbers.begin();
	for (; it3 != col_numbers.end(); it3++) {
		sort_cmd += "-k " + boost::lexical_cast<string>(*it3) + "," + boost::lexical_cast<string>(*it3) + " ";
	}
	sort_cmd += get_file_path() + " -o " + get_file_path();
	f_write->close();
	system(sort_cmd.c_str());
	reset();
	return *this;
}

Table Table::union_(Table t) {
	if (!compatible(t))
		throw TABLE_ERROR("Incompatible tables can't be unioned");
	Table unioned(name + "union" + t.name, attr_names, attr_types);
	reset();
	t.reset();
	Tuple t1, t2;
	vector<string>::iterator it1, it2;
	while (!end_of_table()) {
		t1 = next_tuple();
		unioned.insert_tuple(t1);
	}
	while (!t.end_of_table()) {
		t1 = t.next_tuple();
		it1 = attr_names.begin();
		it2 = t.attr_names.begin();
		for (; it1 != attr_names.end(); it1++, it2++) {
			t2.insert(pair<string, boost::any>(*it1, t1[*it2]));
		}
		unioned.insert_tuple(t2);
		t2.clear();
	}
	return unioned;
}

Table Table::intersection(Table t) {
	if (!compatible(t))
		throw TABLE_ERROR("Incompatible tables can't be intersected");
	Table intersected(name + "intersection" + t.name, attr_names, attr_types);
	reset();
	Tuple t1, t2;
	while (!end_of_table()) {
		t1 = next_tuple();
		t.reset();
		while (!t.end_of_table()) {
			t2 = t.next_tuple();
			if (same_type_insensitive(t1, attr_names, t2, t.attr_names)) {
				intersected.insert_tuple(t1);
				break;
			}
		}
	}
	return intersected;
}

Table Table::set_difference(Table t) {
	if (!compatible(t))
		throw TABLE_ERROR("Incompatible tables can't be subtracted");
	Table diffed(name + "diff" + t.name, attr_names, attr_types);
	reset();
	Tuple t1, t2;
	bool found;
	while (!end_of_table()) {
		t1 = next_tuple();
		t.reset();
		found = false;
		while (!t.end_of_table()) {
			t2 = t.next_tuple();
			if (same_type_insensitive(t1, attr_names, t2, t.attr_names)) {
				found = true;
				break;
			}
		}
		if (!found)
			diffed.insert_tuple(t1);
	}
	return diffed;
}

void Table::print() {
	Tuple t;
	vector<string>::iterator it0 = attr_names.begin();
	for (; it0 != attr_names.end(); it0++) {
		cout << *it0 << "   ";
	}
	cout << endl;
	reset();
	try {
		while (!end_of_table()) {
			t = next_tuple();
			it0 = attr_names.begin();
			for (; it0 != attr_names.end(); it0++) {
				generic_print(attr_type_map[*it0], t[*it0]);
				cout << "   ";
			}
			cout << "\n";
		}
	} catch (...) {
		cout << "End of table reached\n";
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
	return tuples.size() == 0 || tuples_read >= tuples.size();
}

bool Table::end_of_table() {
	return eof && buffer_empty();
}

string Table::get_file_path() {
	return file.string();
}

string Table::get_md_file_path() {
	return md_file.string();
}

void Table::make_permanent() {
	string random_dir = random_str_gen(8);
	path dest_dir("./db/" + random_dir);
	create_directory(dest_dir);
	path dest = dest_dir/file.filename();
	copy_file(file, dest);
	file = dest;
	dest = dest_dir/md_file.filename();
	copy_file(md_file, dest);
	md_file = dest;
}

Tuple Table::line_to_tuple(const string &line) {
	istringstream ss(line);
	string token;
	vector<string> values;
	while (getline(ss, token, ';')) {
		//cout << "read " << token << endl;
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

bool Table::compatible(Table t) {
	if (attr_type_map.size() != t.attr_type_map.size())
		return false;
	vector<string>::iterator it1, it2;
	for (it1 = attr_names.begin(), it2 = t.attr_names.begin(); it1 != attr_names.end(); it1++, it2++) {
		if (attr_type_map[*it1] != t.attr_type_map[*it2])
			return false;
	}
	return true;
}

static bool satisfies_natural_join(const Tuple &t1, const Tuple &t2, const vector<string> &common_attrs) {
	Tuple::const_iterator it2, it3;
	for (vector<string>::const_iterator it = common_attrs.begin(); it != common_attrs.end(); it++) {
		it2 = t1.find(*it);
		it3 = t2.find(*it);
		if (it2 == t1.end() || it3 == t2.end())
			return false;
		if (it2->second.type() != it3->second.type())
			return false;
		if (it2->second.type() == typeid(string))
			if (boost::any_cast<string>(it2->second) != boost::any_cast<string>(it3->second))
				return false;
		else if (it2->second.type() == typeid(int))
			if (boost::any_cast<int>(it2->second) != boost::any_cast<int>(it3->second))
				return false;
		else
			return false;
	}
	return true;
}

static bool check_truth(boost::any a, boost::any b, int cond) {
	//cout << "entered" <<  cond << "\n";
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

static string random_str_gen(int length) {
    static string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    string result;
    result.resize(length);
    for (int i = 0; i < length; i++)
        result[i] = charset[rand() % charset.length()];

    return result;
}

static bool same(const Tuple &t1, const Tuple &t2, const vector<string> &attrs) {
	vector<string>::const_iterator it = attrs.begin();
	Tuple::const_iterator it2, it3;
	for (; it != attrs.end(); it++) {
		it2 = t1.find(*it);
		it3 = t2.find(*it);
		if (it2 == t1.end() || it3 == t2.end())
			return false;
		if ((it2->second).type() != (it3->second).type())
			return false;
		if (it2->second.type() == typeid(string)) {
			//cout << *it << " " << boost::any_cast<string>(it3->second) << " " << boost::any_cast<string>(it3->second) << endl;
			if (boost::any_cast<string>(it2->second) != boost::any_cast<string>(it3->second))
				return false;
		}
		else {
			if (boost::any_cast<int>(it2->second) != boost::any_cast<int>(it3->second))
				return false;
		}
	}
	return true;
}

static bool same(const Tuple &t1, const Tuple &t2) {
	Tuple::const_iterator it, it2;
	if (t1.size() != t2.size())
		return false;
	for (it = t1.begin(); it != t1.end(); it++) {
		it2 = t2.find(it->first);
		if (it2 == t1.end())
			return false;
		if ((it2->second).type() != (it->second).type())
			return false;
		if (it2->second.type() == typeid(string)) {
			if (boost::any_cast<string>(it2->second) != boost::any_cast<string>(it->second))
				return false;
		}
		else {
			if (boost::any_cast<int>(it2->second) != boost::any_cast<int>(it2->second))
				return false;
		}
	}
	return true;
}

static bool same_type_insensitive(const Tuple &t1, const vector<string> &attr_names_1,
								  const Tuple &t2, const vector<string> &attr_names_2) {
	if (t1.size() != t2.size())
		return false;
	vector<string>::const_iterator it1 = attr_names_1.begin();
	vector<string>::const_iterator it2 = attr_names_2.begin();
	for (; it1 != attr_names_1.end(); it1++, it2++) {
		boost::any a = (t1.find(*it1))->second;
		boost::any b = (t2.find(*it2))->second;
		if (b.type() != a.type())
			return false;
		if (b.type() == typeid(string))
			if (boost::any_cast<string>(a) != boost::any_cast<string>(b))
				return false;
		else if (b.type() == typeid(int))
			if (boost::any_cast<int>(a) != boost::any_cast<int>(b))
				return false;
		else return false;
	}
	return true;
}
