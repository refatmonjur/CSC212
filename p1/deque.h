#pragma once

#include <cstdio> /* for FILE* and size_t */

/* declaration of our deque class.  We'll just store integers. */
class deque
{
public:
	/* constructors.  called when a variable of this type is created.  */
	deque(); /* default constructor; should give an empty deque */
	deque(int* first, int* last); /* initialize from [last..first) */
	deque(const deque& D); /* copy constructor */
	deque(deque&& D); /* r-value copy constructor */

	deque& operator=(deque RHS); /* assignment operator */

	/* destructor. this is called when the variable is freed
	 * (i.e., when it goes out of scope, or explicitly deleted).  */
	~deque();
	/* NOTE: pretty much *anytime* you are making a class / struct that
	 * manages dynamic memory, you need to write all of the above functions,
	 * as the default behavior will not suffice. */

	/********  other member functions  ********/

	/* push_{front,back}() functions.  */
	void push_back(int);
	void push_front(int);

	/* pop_{front,back}() functions.
	 * Pre-condition: deque is not empty
	 * Post-condition: element at the front of the deque is removed
	 * Return value: the element at the front of the deque
	 * prior to the function call is returned.
	 * */
	int pop_back();
	int pop_front();

	/* return true iff deque is empty.  */
	bool empty() const;

	int& operator[](size_t i) const;

	/* back() function.
	 * Pre-condition: deque is not empty.
	 * Post-condition: deque is unaffected.
	 * Return value: the element at the back of the deque is returned.
	 * */
	int back() const;

	/* front() function.
	 * Pre-condition: deque is not empty.
	 * Post-condition: deque is unaffected.
	 * Return value: the element at the front of the deque is returned.
	 * */
	int front() const;

	/* returns the number of elements in the deque. */
	size_t size() const;

	/* returns the maximal number of elements that could be stored without
	 * resizing the underlying buffer. */
	size_t capacity() const;
	/* NOTE: this is not a member function of std::deque, as it is
	 * implemented differently behind the scenes. */

	/* clear function.
	 * Pre-condition: none.
	 * Post-condition: deque is empty.
	 * */
	void clear();

	/* for convenience, a print function */
	void print(FILE* f) const; /* f should be open... */

private:
	/* NOTE: the private section is for the "behind the scenes"
	 * implementation details.  No one needs to know what's here
	 * in order to make use of our data structure. */
	int* data; /* pointer to storage for elements */
	size_t front_index; /* index of the front element */
	size_t next_back_index; /* index _after_ the back element */
	size_t cap; /* size of buffer pointed to by data */
	/* NOTE: cap is one greater than the maximal number of elements
	 * stored in the deque (if all spaces are used, it is not possible
	 * to distinguish full from empty...) */
	bool needs_realloc(); /* can we push another element with just the
							 existing storage? */
};
