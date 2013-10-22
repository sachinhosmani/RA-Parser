#ifndef TABLE_H
#define TABLE_H

#include <map>
#include <vector>
#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <iostream>
#include "predicate.h"

using namespace std;

typedef map<string, boost::any> Tuple;

class Table {
public:
	vector<Tuple> tuples;
	vector<string> attr_names;
	vector<string> attr_types;
	map<string, string> attr_type_map;
	string name;
	Table();
	Table(const string &a_name, const vector<string> &a_attr_names, const vector<string> &a_attr_types);
	void insert_tuple(const Tuple &t);
	void insert_tuple(const vector<string> &values);
	Table cross(const Table &t) const;
	Table project(const vector<string> &a_attr_names);
	Table select(Predicate *p);
	void print();
	static bool satisfies(Predicate *p, const Tuple &t);
	static bool satisfies(Simple_Predicate *p, const Tuple &t);
	static boost::any parse_e_tree(Expression_Tree *e, const Tuple &t);
};

class TABLE_ERROR {
public:
	string msg;
	inline TABLE_ERROR(const string &a_msg):msg(a_msg){};
};

static bool check_truth(boost::any a, boost::any b, int cond);
static bool is_string_literal(const string &s);
static bool is_int_literal(const string &s);
#endif
