/* simple test program for BST. */

#include <cstdio>
#include <getopt.h> /* parse long arguments */
#include <stdlib.h> /* for mkdtemp */
#include "tree.hpp"

static const char* usage =
"Usage: %s [OPTIONS]...\n"
"Apply sequence of operations to a binary search tree.\n"
"Reads stdin, writes stdout.\n\n"
"OPTIONS SUMMARY:\n"
"   -c,--copyconst     run copy constructor / operator= test and exit.\n"
"   -h,--help          show this message and exit.\n\n"
"OPERATIONS SUMMARY:\n"
"(i)nsert (r)emove (s)earch (p)reorder (P)ostorder (I)norder\n"
"(S)ize (l)eaves (h)eight (m)in (M)ax (c)lear (d)raw an SVG\n\n"
"EXAMPLE INPUT:\n\n"
"i 23\ni 42\ni 19\np\nd\n\n"
"The above would insert the three integers into a tree,\n"
"output a preorder traversal (resulting in 23 19 42), and\n"
"draw the tree (SVG format) in /tmp/trees-XXXXXX/tN.svg\n"
"where N is the (1-based) index of the command in the input.\n";

/* print a node according to a format string.
 * precondition: n is not NULL. */
template <typename K>
void printNode(const node<K>* const& n, void* fmt)
{
	/* the void* param is expected to be a format string with a single %i
	 * (for the data of the node) and no other format specifiers... */
	printf((char*)fmt,n->data);
}

template <typename K>
void printPreorder(const tree<K>& T)
{
	preorder(T.rootptr(),&printNode,(void*)"%lu ");
	printf("\n");
}
template <typename K>
void printInorder(const tree<K>& T)
{
	inorder(T.rootptr(),&printNode,(void*)"%lu ");
	printf("\n");
}
template <typename K>
void printPostorder(const tree<K>& T)
{
	postorder(T.rootptr(),&printNode,(void*)"%lu ");
	printf("\n");
}

/* for writing in dot format: */
template <typename K>
void node2dot(const node<K>* const& n, void* pFILE)
{
	/* NOTE: the void* param is assumed to be a file stream which
	 * is opened for writing.  Also, you need to do this post order. */
	if (n==0) return;
	FILE* f = (FILE*)(pFILE);
	fprintf(f, "  \"%p\" [label=%lu]\n", n,n->data);
	if (!(n->left||n->right)) return;
	/* nodes with a single child may be rendered directly below, so
	 * print invisible nodes to space it out: */
	if (!n->left) {
		fprintf(f, "  \"%s%p\" [label=\"\",width=.1,style=invis]","l",n);
		fprintf(f, "  \"%p\" -> \"%s%p\" [style=invis]\n",n,"l",n);
	} else {
		fprintf(f, "  \"%p\" -> \"%p\" [style=dashed]\n",n,n->left);
	}
	if (!n->right) {
		fprintf(f, "  \"%s%p\" [label=\"\",width=.1,style=invis]","r",n);
		fprintf(f, "  \"%p\" -> \"%s%p\" [style=invis]\n",n,"r",n);
	} else {
		fprintf(f, "  \"%p\" -> \"%p\"\n",n,n->right);
	}
}

/* In case you want to change colors: */
const char* nodefill  = "darkolivegreen3";
const char* bgcolor   = "black";
const char* edges     = "white";
/* choices here: https://www.graphviz.org/doc/info/colors.html */

template <typename K>
void drawSVG(const tree<K>& T, const char* fname)
{
	char dotcmd[512];
	snprintf(dotcmd,511,"dot -Tsvg -o '%s'",fname);
	FILE* fsvg = popen(dotcmd,"w");
	if (fsvg == 0) {
		fprintf(stderr, "couldn't subprocess \"%s\"\n",dotcmd);
		return;
	}
	fprintf(fsvg, "digraph BST {\n");
	fprintf(fsvg, "  graph [ordering=\"out\"];\n");
	fprintf(fsvg, "  bgcolor=%s\n  edge [color=%s]\n",bgcolor,edges);
	fprintf(fsvg, "  node [style=filled color=%s fillcolor=%s shape=circle]\n",
			edges, nodefill);
	postorder(T.rootptr(),&node2dot,fsvg);
	fprintf(fsvg, "}\n");
	int status = pclose(fsvg);
	if (status == -1) {
		fprintf(stderr, "pclose failed.\n");
	}
}

void ccTest()
{
	/* make a tree with random stuff; make copies; compare contents;
	 * see if anything crashes. */
	tree<size_t> T;
	size_t p = 61819;
	size_t g = 26230;
	size_t x = g;
	for (size_t i = 0; i < 100; i++) {
		T.insert(x);
		// fprintf(stderr, "%lu\n",x);
		x*=g;
		x%=p;
	}
	printPreorder(T);
	/* check copy constructor: */
	tree<size_t> T2(T);
	printPreorder(T2);
	/* make a 3rd tree and overwrite with T */
	tree<size_t> T3;
	for (size_t i = 0; i < 100; i++) {
		T3.insert(x); /* NOTE: state of x preserved from before */
		x*=g;
		x%=p;
	}
	// printPreorder(T3);
	T3 = T;
	printPreorder(T3);
	/* now just check that all lines of output are identical... */
}

int main(int argc, char *argv[])
{
	/* process command line: */
	static struct option long_opts[] = {
		{"copyconst", no_argument,       0, 'c'},
		{"help",      no_argument,       0, 'h'},
		{0,0,0,0}
	};
	char c;
	int opt_index = 0;
	while ((c = getopt_long(argc, argv, "hc", long_opts, &opt_index)) != -1) {
		switch (c) {
			case 'c':
				ccTest();
				return 0;
			case 'h':
				printf(usage,argv[0]);
				return 0;
			case '?':
				printf(usage,argv[0]);
				return 1;
		}
	}

	/* process stdin: */
	char input;
	size_t x;
	tree<size_t> T;
	bool madetmp = false; /* did we invoke mkdtemp for svg files? */
	char tmpdir[64] = "/tmp/trees-XXXXXX";
	char fname[256];
	size_t count = 0; /* count of commands */

	const char* yn = "\0not ";
	while((scanf(" %c",&input)) != EOF)
	{
		count++;
		switch(input) {
		case 'i': /* insert */
			if (scanf("%lu",&x) == 1) {
				T.insert(x);
			} else {
				fprintf(stderr,
						"Parse error reading input (src:%d,input:%lu).\n",
						__LINE__,count);
				return 2;
			}
			break;
		case 'r': /* remove */
			if (scanf("%lu",&x) == 1) {
				T.remove(x);
			} else {
				fprintf(stderr,
						"Parse error reading input (src:%d,input:%lu).\n",
						__LINE__,count);
				return 2;
			}
			break;
		case 's': /* search */
			if (scanf("%lu",&x) == 1) {
				printf("%lu was %sin the tree.\n",x,yn+(int)!T.search(x));
			} else {
				fprintf(stderr,
						"Parse error reading input (src:%d,input:%lu).\n",
						__LINE__,count);
				return 2;
			}
			break;
		case 'p': /* preorder */
			printPreorder(T);
			break;
		case 'P': /* postorder */
			printPostorder(T);
			break;
		case 'I': /* inorder */
			printInorder(T);
			break;
		case 'S': /* size */
			printf("#nodes == %lu\n",T.size());
			break;
		case 'l': /* leaves */
			printf("#leaves == %lu\n",T.leaves());
			break;
		case 'h': /* height */
			printf("height == %lu\n",T.height());
			break;
		case 'm': /* min */
			printf("min(T) == %lu\n",T.min());
			break;
		case 'M': /* max */
			printf("max(T) == %lu\n",T.max());
			break;
		case 'c': /* clear */
			T.clear();
			break;
		case 'd':
			/* make temp directory if not already there: */
			if (!madetmp) {
				mkdtemp(tmpdir);
				madetmp = true;
			}
			snprintf(fname,256,"%s/t%lu.svg",tmpdir,count);
			drawSVG(T,fname);
			break;
		default:
			fprintf(stderr,
					"Parse error reading input (src:%d,input:%lu).\n",
					__LINE__,count);
			return 2;
		}
	}
	/* upon normal exit, let user know where to find svg files: */
	if (madetmp) {
		fprintf(stderr, "svg output written to %s\n",tmpdir);
	}
	return 0;
}
