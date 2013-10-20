#ifndef TABLE_H
#define TABLE_H

#include <map>
#include <vector>
#include <boost/any.hpp>
#include <string>
#include <iostream>

using namespace std;

typedef map<string, boost::any> Tuple;

class Table {
public:
	vector<Tuple> tuples;
	vector<string> attr_names;
	map<string, string> attr_type_map;
	string name;
	Table();
	Table(const string &a_name, const vector<string> &a_attr_names, const vector<string> &a_attr_types);
	void insert_tuple(const Tuple &t);
	void insert_tuple(const vector<string> &values);
	Table cross(const Table &t) const;
	Table project(const vector<string> &a_attr_names);
	void print();
};

class TABLE_ERROR {
public:
	string msg;
	inline TABLE_ERROR(const string &a_msg):msg(a_msg){};
};
#endif
