/******************************************************************
This is the header file for our hash table.  We'll implement this
data structure via an array of linked lists (aka a chained hash
table).  You can read about this structure at the end of chapter 12
of the textbook, but they do not give an implementation there.

We'll formalize our representation of the data via a class invariant
as usual:

Class invariant: 
1. the complete contents of the structure are simply the union over
   all of the linked lists in the array.  furthermore...
2. an element with key == x is in the hash table if and only if it
   is in the list with index h(x), for our hash function h.
3. the hash table will store unique elements, so repeated insertion
   of the same element should only have an effect on the first attempt.

Note on 3: the linked list does not store unique elements, so you'll
want to search the list before you perform the insertion.
Note on the hash function h: this is selected randomly from an (almost)
universal hashing family during the object's construction.
*********************************************************************/

#pragma once

#include <iostream>
using std::ostream;
#include <string>
using std::string;
#include <list>
using std::list;
#include <inttypes.h>

namespace csc212 {
	/* NOTE: to use this with other types requires adding overloads
	 * to the hash function. */
	typedef std::string val_type;

	class hashTbl
	{
	public:
		/* construction / destruction */
		hashTbl(unsigned nBits = 8, const uint32_t* ha = 0,
					const uint64_t* halpha = 0, const uint64_t* hbeta = 0);
		/* NOTE: the last 3 parameters would generally not be set: this is
		 * more for testing (so we can make experiments repeatable) even
		 * across platforms. */
		hashTbl(const hashTbl& H);
		~hashTbl();

		/* operators: */
		hashTbl& operator=(hashTbl H); //assignment
		friend ostream& operator<<(ostream& o, const hashTbl& H);

		/* other member functions: */
		/* pre: none/suff. memory
		 * post: x is inserted into the hash table */
		void insert(val_type x);

		/* pre: none
		 * post: x is removed from the table if present */
		void remove(val_type x);

		/* pre: none
		 * post: all elements are removed from the table */
		void clear();

		/* pre: none
		 * post: table is unaffected
		 * return: true iff the hash table is empty */
		bool isEmpty() const;
		
		/* pre: none
		 * post: table is unaffected
		 * return: boolean that is true iff x was found */
		bool search(val_type x) const;

		/* pre: none
		 * post: table is unaffected
		 * return: the total number of elements in the table */
		size_t countElements() const;

		/* pre: none
		 * post: table is unaffected
		 * return: number of slots in the hash table */
		size_t tableLength() const;

		/* pre: none
		 * post: table is unaffected
		 * return: count the number of collisions: for simplicity, we'll just
		 * count the number of lists in our table with a length > 1. */
		size_t countCollisions() const;

		/* pre: none
		 * post: table is unaffected
		 * return: the length of the longest list in the hash table. */
		size_t longestListLength() const;

		/* NOTE: this should really be in the private section; it is
		 * made public so that the test program can access it directly. */
		class hash {
			public:
			/* see the readme for these parameters.  if the pointer
			 * params are null, then the constructor should randomly
			 * assign them.  the hash function will have codomain
			 * 0,...,2^rangebits - 1 */
			hash(unsigned rangebits = 64, const uint32_t* a = 0,
					const uint64_t* alpha = 0, const uint64_t* beta = 0);
			uint64_t operator()(const string& x) const;
			static const size_t aLen = 32; /* we can only hash 128 byte strings */
			unsigned rangebits;
			uint32_t a[aLen]; /* vector for initial hash */
			uint64_t alpha; /* for second hash */
			uint64_t beta; /* for second hash */
		};
	private:
		list<val_type>* table; /* array to hold the linked lists */
		unsigned nBits; /* size of hash table == 2^nBits */
		hash h; /* instance of hash function */
	};
}
