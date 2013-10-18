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
		}
		crossed.insert_tuple(tmp);
		tmp.clear();
	}
	return crossed;
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
