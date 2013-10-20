#include "predicate.h"

int predicate_op_sym_to_code(const string &op_sym) {
	for (int = 0; i < PREDICATE_OP_COUNT; i++) {
		if (op_sym == PREDICATE_OP_SYMBOLS[i])
			return i + 1;
	}
	return 0;
}

int cond_op_sym_to_code(const string &op_sym) {
	for (int = 0; i < COND_OP_COUNT; i++) {
		if (op_sym == COND_OP_SYMBOLS[i])
			return i + 1;
	}
	return 0;
}

Predicate *create_predicate(const string &p_query) {
	Predicate *p = new Predicate;
	Predicate *first;
	string op;
	Tokenizer t(p_query);
	string token;
	string until = "";
	int b_ctr = 0;
	while (!t.eof()) {
		token = t.next_token();
		if (token == "(") {
			b_ctr++;
			if (b_ctr != 1)
				until += token;
			continue;
		}
		else if (token == ")") {
			b_ctr--;
			if (!t.eof())
				until += token;
			continue;
		}
		int maybe_op = predicate_op_sym_to_code(token);
		if (b_ctr == 1 && (maybe_op > 0)) {
			op = maybe_op;
			first = create_predicate(until);
			until = "";
		} else {
			until += token;
		}
	}
	if (op == "") {
		t->first = NULL;
		t->rest = NULL;
		t->sp = create_simple_predicate(until);
		t->op = 0;
	}
	else {
		t->first = first;
		t->rest = create_predicate(until);
		t->op = op;
		t->sp = NULL;
	}
	return p;
}

Simple_Predicate *create_simple_predicate(const string &sp_query) {
	Simple_Predicate *sp = new Simple_Predicate;
	Tokenizer t(sp_query);
	string left = "";
	string right = "";
	int op = 0;
	string token;
	while (!t.eof()) {
		token = t.next_token();
		int maybe_op = cond_op_sym_to_code(token)
		if (maybe_op > 0) {
			op = maybe_op;
		}
		else {
			if (op > 0)
				right += token;
			else
				left += token;
		}
	}
	if (op <= 0)
		// throw something
	sp->left = create_expression_tree(left);
	sp->right = create_expression_tree(right);
	sp->op = op;
	return sp;
}

Expression_Tree *create_expression_tree(const string &e_query) {
	Expression_Tree *e = new Expression_Tree;
	Expression_Tree *first;
	string data = "";
	string token;
	string until = "";
	int b_ctr = 0;
	while (!t.eof()) {
		token = t.next_token();
		if (token == "(") {
			b_ctr++;
			if (b_ctr != 1)
				until += token;
			continue;
		}
		else if (token == ")") {
			b_ctr--;
			if (!t.eof())
				until += token;
			continue;
		}
		bool is_op = is_expression_op(token);
		if (b_ctr == 1 && is_op) {
			op = token;
			first = create_expression_tree(until);
			until = "";
		} else {
			until += token;
		}
	}
	if (op == "") {
		t->left = NULL;
		t->right = NULL;
		t->data = 0;
	}
	else {
		t->left = first;
		t->right = create_predicate(until);
		t->data = predicate_op_sym_to_code(op);
	}
	return e;
}
