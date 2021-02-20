/* Simple binary search tree. */  //check out "boost" c++ libraries for examples, for advance use of the template
#pragma once
#include <cstddef> /* for size_t */
#include <cassert>

/* here's how to make your own templates: just prefix the declaration with
 * "template" followed by a list of type parameters.  E.g. for a tree node
 * we might use the following.  Note that 'K' is a *type* variable, so we
 * can declare things of type 'K' in the template. */
template <typename K>
struct node {
	K data;
	node<K>* left;
	node<K>* right;
	node(K d=K(), node* l=NULL, node* r=NULL) : data(d),left(l),right(r) {} //node<K> constructor
};

template <typename K>
class tree {
public:
	/* basic stuff: constructors, destructors, assignment... */
	tree();
	tree(const tree<K>& T);
	tree<K>& operator=(tree<K> T);
	~tree();

	bool search(K x) const;
	void insert(K x);
	void remove(K x);
	void clear(); /* remove all elements */

	/* * * * * * * statistics * * * * * * */
	K max() const;         /* maximal value */
	K min() const;         /* minimal value */
	size_t size() const;   /* total number of nodes */
	size_t height() const; /* longest path from root to leaf. */
	size_t leaves() const; /* returns number of leaf nodes */

	/* NOTE: we include a function that retuns the location of the root
	 * node, but we do so *by value*, and furthermore we make the pointee
	 * const.  Hence, without really going out of one's way, the state of the
	 * tree should not be easily modified via non-member functions (none of
	 * the internal pointers can be changed).  However, one could still do
	 * something ridiculous, like calling delete on one of the nodes, I
	 * guess.  But note that such things are present in the STL as well.
	 * E.g., you can get a copy of a vector's pointer to its (nominally
	 * behind the scenes) array simply by storing &V[0]. */
	node<K>* const rootptr() const;
	/* NOTE: our main application for this feature is that it allows us
	 * to draw the tree in a variety of formats (see main.cpp).  It's not
	 * clear how useful this is in general for a BST, but I hope you find
	 * it to have educational value.  */
private:
	node<K>* root;
};

/* NOTE: you must include, one way or another, the implementation right
 * along with the header.  We will put our implementation here in the same
 * file (named *.hpp) below.  To understand why, think about the following
 * question: what object code could the compiler possibly generate by
 * compiling a bunch of template functions and structs??
 * TODO: make sure you have a satisfactory understanding of this before
 * moving on to ANY of the programming.
 * */


/* to avoid typing out the template params for every function: */
#define TK template <typename K>

/* * * * * * * * * * basic dynamic set operations * * * * * * * * * */
TK void insert(node<K>*& n, K x)
{
	/* TODO: write me */
	if (n == NULL) {
		n = new node<K> (x);
		return;
	}
	else if (x < n->data) {
		insert(n->left, x);
	}
	else if (x > n->data) {
		insert(n->right, x);
	}
}

TK bool search(const node<K>* const& n, K x)
{
	/* TODO: write me */
	if (n == NULL)
		return false;
	else if (x < n->data)
		search(n->left, x);
	else if (x > n->data)
		search(n->right, x);
	else if (n->data == x)
		return true;
/* avoid compilation errors/warnings */
}

TK void remove(node<K>*& n, K x)
{
	/* TODO: write me */
	if (n == NULL)
		return;
	else if (x < n->data)
		remove(n->left, x);
	else if (x > n->data)
		remove(n->right, x);
	else {
		if (n->left == NULL && n->right == NULL) {
			delete n;
			n = NULL;
		}
		else if (n->left == NULL) {
			node<K>* temp = n;
			n = n->right;
			delete temp;
		}
		else if (n->right == NULL) {
			node<K>* temp = n;
			n = n->left;
			delete temp;
		}
		else {
			node<K>* v = n->right;
			while (v->left)
				v = v->left;
			n->data = v->data;
			remove(n->right, v->data);
		}
	}
}

/* the tree's member functions will just hand off to those acting on
 * nodes (see above): */
TK void tree<K>::insert(K x)
{
	::insert(this->root,x);
}
TK bool tree<K>::search(K x) const
{
	return ::search(this->root,x);
}
TK void tree<K>::remove(K x)
{
	::remove(this->root,x);
}


/* "function pointer" type for things that operate on nodes: */
template <typename K>
using nodeProcFunc = void (*)(node<K>*& n, void* pParams);
/* NOTE: the above says that any function which accepts a node<K>*
 * by reference and returns void counts as a "nodeProcFunc".  Before,
 * we did this with a typedef statement, but that's no longer possible
 * once templates are involved.  */
/* version for const input: */
template <typename K>
using c_nodeProcFunc = void (*)(const node<K>* const& n, void* pParams);

/* * * * * * * * * * tree traversal functions * * * * * * * * * */
TK void preorder(node<K>*& n, nodeProcFunc<K> f, void* pParams)
{
	/* TODO: write me */
	if (n == NULL) return;
	f(n, pParams);
	preorder(n->left, f, pParams);
	preorder(n->right, f, pParams);
}
TK void inorder(node<K>*& n, nodeProcFunc<K> f, void* pParams)
{
	/* TODO: write me */
	if (n == NULL) return;
	inorder(n->left, f, pParams);
	f(n, pParams);
	inorder(n->right, f, pParams);
}
TK void postorder(node<K>*& n, nodeProcFunc<K> f, void* pParams)
{
	/* TODO: write me */
	if (n == NULL) return;
	postorder(n->left, f, pParams);
	postorder(n->right, f, pParams);
	f(n, pParams);
}
/* * * * * * * * * * traversal functions (const) * * * * * * * * * */
TK void preorder(const node<K>* const& n, c_nodeProcFunc<K> f, void* pParams)
{
	/* TODO: write me */
	/* NOTE: text of this function should be the same as the non-const */
	if (n == NULL) return;
	f(n, pParams);
	preorder(n->left, f, pParams);
	preorder(n->right, f, pParams);
}
TK void inorder(const node<K>* const& n, c_nodeProcFunc<K> f, void* pParams)
{
	/* TODO: write me */
	/* NOTE: text of this function should be the same as the non-const */
	if (n == NULL) return;
	inorder(n->left, f, pParams);
	f(n, pParams);
	inorder(n->right, f, pParams);
}
TK void postorder(const node<K>* const& n, c_nodeProcFunc<K> f, void* pParams)
{
	/* TODO: write me */
	/* NOTE: text of this function should be the same as the non-const */
	if (n == NULL) return;
	postorder(n->left, f, pParams);
	postorder(n->right, f, pParams);
	f(n, pParams);
}


/* * * * * * * * * * clear(), destructor * * * * * * * * * */
TK void deleteTree(node<K>*& n) {
	if (n == NULL) return;
	deleteTree(n->left);
	deleteTree(n->right);
	delete n;
	n = NULL;
}
TK tree<K>::~tree()
{
	/* TODO: write me */
	::deleteTree(this->root);
}



TK void tree<K>::clear()
{
	/* TODO: write me */
	/* NOTE: can you do this via a traversal? */
	node<K>* temp = this->root;
	this->root = NULL;
}

/* * * * * * * * * * constructors, assignment * * * * * * * * * */
/* default constructor: make an empty tree: */
TK tree<K>::tree()
{
	this->root = NULL;
}


TK void copySubtree(node<K>*const& n, node<K>*& a) {
	if (n == NULL)
		return;
	a = new node<K> (n->data);
	copySubtree(n->left, a->left);
	copySubtree(n->right, a->right);
}
TK tree<K>::tree(const tree<K>& T)
{
	/* TODO: write me */
	/* NOTE: maybe use a function like this:
	 * TK node<K>* copySubtree(const node<K>* const& s);
	 * and then use it to set this->root...
	 * */
	::copySubtree(T.root, this->root);
}

TK tree<K>& tree<K>::operator=(tree<K> T)
{
	/* TODO: write me */
	/* NOTE: the plan is to let the copy constructor do the real work
	 * and we'll just steal it here. */
	node<K>* temp = this->root;
	this->root = T.root;
	T.root = temp;
	return *this;
}

/* * *  * * * * * * statistics * * * * * * * * */
TK void size(const node<K>* const& n, size_t& s) {
	if (n == NULL)
		return;
	else
		s++;
	size(n->left, s);
	size(n->right, s);
}
TK size_t tree<K>::size() const
{
	/* TODO: write me */
	/* NOTE: you can make a function that acts on nodes like this:
	 * TK size_t size(const node<K>* const& n);
	 * Or perhaps solve this via a traversal + a small function that
	 * is of type c_nodeProcFunc...
	 * */
	size_t num = 0;
	::size(this->root, num); /* avoid compiler errors/warnings */
	return num;
}



TK void leafNode(const node<K>* const& n, size_t& l) {
	if (n == NULL)
		return;
	else
		if(n->left==NULL && n->right ==NULL)
			l++;
	leafNode(n->left, l);
	leafNode(n->right, l);
}
TK size_t tree<K>::leaves() const
{
	/* TODO: write me.  See remarks above for size */
	size_t l = 0;
	::leafNode(this->root,l); /* avoid compiler errors/warnings */
	return l;
}



TK size_t height(const node<K>* const& n) {
	if (n == NULL) {
		return 0;
	}
	else{
	size_t l = height(n->left);
	size_t r = height(n->right);
	if(l>r)
		return (l+1);
	else
		return (r+1);
	}
}
TK size_t tree<K>::height() const
{
	/* TODO: write me.  Maybe write function like this to help:
	 * TK size_t height(const node<K>* const& n);
	 * NOTE: we use the convention that the empty tree has height -1.
	 * */
	return ::height(this->root)-1; /* avoid compiler errors/warnings */

}




TK K tree<K>::min() const
{
	assert(this->root != NULL);
	/* TODO: write me. */
	node<K>* temp = this->root;
	while (temp->left)
		temp = temp->left;
	return temp->data; /* avoid compiler errors/warnings */
}

TK K tree<K>::max() const
{
	assert(this->root != NULL);
	/* TODO: write me. */
	node<K>* temp = this->root;
	while (temp->right)
		temp = temp->right;
	return temp->data; /* avoid compiler errors/warnings */
}

/* expose constant pointer to root node, which enables interesting
 * applications (via tree traversals, for example). */
TK node<K>* const tree<K>::rootptr() const
{
	return this->root;
}