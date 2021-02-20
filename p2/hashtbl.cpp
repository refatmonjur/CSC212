/****************************************************
implementation file for the hash table class.
*****************************************************/

#include "hashtbl.h"
#include <iostream>
using std::endl;
using std::ostream;
#include <string.h> /* for strcpy */
#include <algorithm> /* for find */
using std::find;
#include <iomanip>
using std::setfill;
using std::setw;
#include <cassert>

#define R32 (rand()*100003 + rand())
#define R64 (((uint64_t)R32 << 32) + R32)
#define TLEN ((size_t)(1 << this->nBits))

namespace csc212
{
	/* First we implement our hash family */
	hashTbl::hash::hash(unsigned rangebits, const uint32_t* a,
					const uint64_t* alpha, const uint64_t* beta) {
		this->rangebits = rangebits;
		if (a) {
			for (size_t i = 0; i < aLen; i++) {
				this->a[i] = a[i] | 1; /* make sure it is odd. */
			}
		} else {
			for (size_t i = 0; i < aLen; i++) {
				this->a[i] = R32 | 1;
			}
		}
		this->alpha = ((alpha) ? *alpha : R64) | 1;
		this->beta = ((beta) ? *beta : R64);
		/* only keep the low order bits of beta: */
		this->beta &= ((uint64_t)-1)>>(64 - rangebits);
	}
	uint64_t hashTbl::hash::operator()(const string& x) const {
		assert(x.length() <= 4*aLen);
		/* TODO: write the hash function. */
		return 0;
	}

	//constructors:
	hashTbl::hashTbl(unsigned nBits, const uint32_t* ha,
					const uint64_t* halpha, const uint64_t* hbeta) :
		nBits(nBits),h(nBits,ha,halpha,hbeta)
	{
		this->table = new list<val_type>[TLEN];
	}
	hashTbl::hashTbl(const hashTbl& H)
	{
		/* TODO: write this */
		/* NOTE: the underlying linked list class has a working
		 * assignment operator! */
	}

	//destructor:
	hashTbl::~hashTbl()
	{
		delete[] this->table;
		//NOTE: this will call the destructor of each of the linked lists,
		//so there isn't anything else that we need to worry about.
	}

	//operators:
	hashTbl& hashTbl::operator=(hashTbl H)
	{
		/* TODO: write this */
		return *this;
	}

	ostream& operator<<(ostream& o, const hashTbl& H)
	{
		for (size_t i = 0; i < H.tableLength(); i++) {
			o << "[" << setfill('0') << setw(2) << i << "] |";
			for (list<val_type>::iterator j = H.table[i].begin();
					j != H.table[i].end(); j++) {
				o << *j << "|";
			}
			o << endl;
		}
		return o;
	}

	void hashTbl::insert(val_type x)
	{
		/* TODO: write this */
		//Remember to check for uniqueness before inserting.
	}

	void hashTbl::remove(val_type x)
	{
		/* TODO: write this */
	}

	void hashTbl::clear()
	{
		for(size_t i=0; i<TLEN; i++)
			this->table[i].clear();
	}

	bool hashTbl::isEmpty() const
	{
		/* TODO: write this */
		return true; // just so it compiles...
	}

	bool hashTbl::search(val_type x) const
	{
		/* TODO: write this */
		return false; // just so it compiles...
	}

	size_t hashTbl::countElements() const
	{
		/* TODO: write this */
		return 0; // just so it compiles...
	}

	size_t hashTbl::tableLength() const
	{
		return TLEN;
	}

	size_t hashTbl::countCollisions() const
	{
		//just count the number of lists of length > 1
		size_t i,nCollisions=0;
		for(i=0; i<TLEN; i++)
			if(table[i].size() > 1)
				++nCollisions;
		return nCollisions;
	}

	size_t hashTbl::longestListLength() const
	{
		/* TODO: write this */
		return 0; // just so it compiles...
	}
}
