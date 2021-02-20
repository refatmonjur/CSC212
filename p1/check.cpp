#include <stdio.h>
#include <stdlib.h> /* for rand() and friends */
#include <iostream>
using std::cout;

#include "deque.h" /* our own deque */
#include <deque>   /* STL's version */

#define R (rand() & 0xFF)

/* IDEA: make a template function that takes something supporting a deque
 * interface and a RNG seed.  Run it on the STL version and our version
 * and make sure the output is the same.
 * Another idea might be to read a file of commands and apply it to each?
 * */

/* NOTE: T has to support at least the basic deque operations
 * that we have defined in our version. */
template <typename Deq>
void rand_test(unsigned int seed);
/* we'll call rand_test<std::deque>(n) and rand_test<deque>(n)
 * and compare the results. */

/* T needs to support operator[] and size(). */
template <typename T>
void print_container(const T& D);

int main()
{
	rand_test<deque>(23);
	printf("@@@\n");
	rand_test<std::deque<int>>(23);
	return 0;
}

template <typename Deq>
void rand_test(unsigned int seed)
{
	/* we'll use one line of output per test. */
	srand(seed);
	{ printf("=======> push_back (check.cpp, line %i)\n",__LINE__);
		Deq D;
		for (size_t i = 0; i < 3; i++) {
			D.push_back(R);
		}
		print_container(D);
	}
	{ printf("=======> push_front (check.cpp, line %i)\n",__LINE__);
		Deq D;
		for (size_t i = 0; i < 3; i++) {
			D.push_front(R);
		}
		print_container(D);
	}
	{ printf("=======> pop_back (check.cpp, line %i)\n",__LINE__);
		int A[] = {0,1,2,3,4,5,6,7};
		Deq D(A,A+8);
		for (size_t i = 0; i < 4; i++) {
			D.pop_back();
		}
		print_container(D);
	}
	{ printf("=======> pop_front (check.cpp, line %i)\n",__LINE__);
		int A[] = {0,1,2,3,4,5,6,7};
		Deq D(A,A+8);
		for (size_t i = 0; i < 4; i++) {
			D.pop_front();
		}
		print_container(D);
	}
	{ printf("=======> push_{back,front} (check.cpp, line %i)\n",__LINE__);
		/* force container to grow; test combinations */
		Deq D;
		for (size_t i = 0; i < 10; i++) {
			D.push_back(R);
			D.push_front(R);
		}
		print_container(D);
		printf("=======> pop_{back,front} (check.cpp, line %i)\n",__LINE__);
		for (size_t i = 0; i < 5; i++) {
			D.pop_front();
			D.pop_back();
		}
		print_container(D);
		printf("=======> simple stuff (check.cpp, line %i)\n",__LINE__);
		printf("back: %i\n",D.back());
		printf("front: %i\n",D.front());
		printf("size: %lu\n",D.size());
	}
	{ printf("=======> copy constructor1 (check.cpp, line %i)\n",__LINE__);
		int A[] = {0,1,2,3,4,5,6,7};
		Deq D(A,A+8);
		Deq* d = new Deq(D);
		print_container(*d);
		printf("=======> operator= (check.cpp, line %i)\n",__LINE__);
		for (size_t i = 0; i < 5; i++) {
			d->push_back(R);
			d->pop_front();
		}
		D = *d;
		print_container(D);
		delete d;
	}
	{ printf("=======> empty (check.cpp, line %i)\n",__LINE__);
		int A[] = {0,1,2,3,4,5,6,7};
		Deq D(A,A+8);
		/* NOTE: D has an even # of elements, so OK to remove in pairs: */
		while (!D.empty()) {
			D.pop_back();
			D.pop_front();
			print_container(D);
		}
	}
	{ printf("=======> clear (check.cpp, line %i)\n",__LINE__);
		int A[] = {0,1,2,3,4,5,6,7};
		Deq D(A,A+8);
		for (size_t i = 0; i < 5; i++) {
			D.push_front(R);
			D.pop_back();
		}
		print_container(D);
		D.clear();
		print_container(D);
	}
}

template <typename T>
void print_container(const T& D)
{
	for (size_t i = 0; i < D.size(); i++) {
		cout << D[i] << " ";
	}
	cout << "\n";
}
