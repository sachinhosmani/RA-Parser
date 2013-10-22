#include "predicate.h"

int predicate_op_sym_to_code(const string &op_sym) {
	for (int i = 0; i < PREDICATE_OP_COUNT; i++) {
		if (op_sym == PREDICATE_OP_SYMBOLS[i])
			return i + 1;
	}
	return 0;
}

int cond_op_sym_to_code(const string &op_sym) {
	for (int i = 0; i < COND_OP_COUNT; i++) {
		if (op_sym == COND_OP_SYMBOLS[i])
			return i + 1;
	}
	return 0;
}

bool is_expression_op(const string &op) {
	for (int i = 0; i < EXPRESSION_OP_COUNT; i++) {
		if (op == EXPRESSION_OP_SYMBOLS[i])
			return true;
	}
	return false;
}

Predicate *create_predicate(const string &p_query) {
	//cout << "called with " << p_query << endl;
	Predicate *p = new Predicate;
	Predicate *first;
	int op = 0;
	Tokenizer t(p_query);
	string token;
	string until = "";
	int b_ctr = 0;
	while (!t.eof()) {
		token = t.next_token();
		if (token == "(") {
			b_ctr++;
			if (b_ctr != 1)
				until += token + " ";
			continue;
		}
		else if (token == ")") {
			b_ctr--;
			if (!t.eof())
				until += token + " ";
			continue;
		}
		int maybe_op = predicate_op_sym_to_code(token);
		if (b_ctr == 1 && (maybe_op > 0)) {
			op = maybe_op;
			first = create_predicate(until);
			until = "";
		} else {
			until += token + " ";
		}
	}
	if (op == 0) {
		p->left = NULL;
		p->right = NULL;
		p->sp = create_simple_predicate(until);
		p->op = 0;
	}
	else {
		p->left = first;
		p->right = create_predicate(until);
		p->op = op;
		p->sp = NULL;
	}
	return p;
}

Simple_Predicate *create_simple_predicate(const string &sp_query) {
	//cout << "called " << sp_query << endl;
	Simple_Predicate *sp = new Simple_Predicate;
	Tokenizer t(sp_query);
	string left = "";
	string right = "";
	int op = 0;
	string token;
	while (!t.eof()) {
		token = t.next_token();
		int maybe_op = cond_op_sym_to_code(token);
		if (maybe_op > 0) {
			op = maybe_op;
		}
		else {
			if (op > 0)
				right += token + " ";
			else
				left += token + " ";
		}
	}
	if (op <= 0)
		throw PARSE_TREE_ERROR("No comparison operator found in \'" + sp_query + "\'");
	//cout << left << " " << right << endl;
	sp->left = create_expression_tree(left);
	sp->right = create_expression_tree(right);
	sp->cond = op;
	return sp;
}

Expression_Tree *create_expression_tree(const string &e_query) {
	cout << "exp: " << e_query << endl;
	Expression_Tree *e = new Expression_Tree;
	Expression_Tree *first;
	string token, op;
	string until = "";
	int b_ctr = 0;
	Tokenizer t(e_query);
	while (!t.eof()) {
		token = t.next_token();
		if (token == "(") {
			b_ctr++;
			if (b_ctr != 1)
				until += token + " ";
			continue;
		}
		else if (token == ")") {
			b_ctr--;
			if (!t.eof())
				until += token + " ";
			continue;
		}
		bool is_op = is_expression_op(token);
		if (b_ctr == 1 && is_op) {
			op = token;
			first = create_expression_tree(until);
			until = "";
		} else {
			until += token + " ";
		}
	}
	if (op == "") {
		cout << "no op " << until << endl;
		e->left = NULL;
		e->right = NULL;
		e->data = until;
		boost::trim(e->data);
	}
	else {
		cout << "op " << op << endl;
		e->left = first;
		e->right = create_expression_tree(until);
		e->data = op;
	}
	return e;
}
