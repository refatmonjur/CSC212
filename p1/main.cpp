/* simple test program for deque class. */

#include <cstdio>
#include "deque.h"

static const char* prompt =
"\nWhat would you like to do?\n\n"
"(p)ush_front an element\n(P)ush_back an element\n"
"p(o)p_front an element\np(O)p_back an element\n"
"get the (f)ront\nget the (b)ack\n"
"check if deque is (e)mpty\n"
"print (s)ize of deque\nprint (c)apacity of deque\n"
"p(r)int the deque\n(d)elete all elements\n"
"(q)uit the program";

int main(void)
{
	printf("\n\033[32m~~~~~~~ Deque test program ~~~~~~~\033[0m\n");

	char input;
	size_t x;
	deque D; /* empty deque */

	printf("%s\n",prompt);
	while((scanf(" %c",&input)) != EOF)
	{
		switch(input) {
		case 'p': /* push_front */
			printf("Item to push_front: ");
			if (scanf("%lu",&x) == 1) {
				D.push_front(x);
			}
			break;
		case 'P': /* push_back */
			printf("Item to push_back: ");
			if (scanf("%lu",&x) == 1) {
				D.push_back(x);
			}
			break;
		case 'o':
			x = D.front();
			D.pop_front();
			printf("%lu removed from front.\n",x);
			break;
		case 'O':
			x = D.back();
			D.pop_back();
			printf("%lu removed from back.\n",x);
			break;
		case 'f':
			x = D.front();
			printf("%lu is at the front.\n",x);
			break;
		case 'b':
			x = D.back();
			printf("%lu is at the back.\n",x);
			break;
		case 'e':
			if(D.empty()) printf("Deque is empty.\n");
			else printf("Deque is non-empty.\n");
			break;
		case 's':
			printf("Deque contains %lu elements.\n",D.size());
			break;
		case 'c':
			printf("Deque capacity is %lu.\n",D.capacity());
			break;
		case 'r':
			printf("From front to back, contents are:\n");
			D.print(stdout);
			break;
		case 'd':
			D.clear();
			printf("Deque cleared.\n");
			break;
		case 'q':
			return 0;
		default:
			printf("Please select an option from the list.\n");
		}
		printf("%s\n",prompt); /* for the next iteration */
	}
	return 0;
}
