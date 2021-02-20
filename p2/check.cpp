#include <stdio.h>
#include <fstream>
using std::ofstream;
using std::endl;

#include <iostream>
using std::cout;
using std::cerr;

#include <string.h>
#include <string>
using std::string;
#include <set>
using std::set;

#include "hashtbl.h"
using namespace csc212;

#ifndef DEBUG
#define DEBUG 0
#endif
#define dbgprint(msg) \
	do { if (DEBUG) fprintf(stderr, "%s", msg); } while(0)
#define dumptbl(T) \
	do { if (DEBUG) cerr << #T"~~~~~~~\n" << T; } while(0)
#define log(op,T) do { op; dumptbl(T); dbgprint("---  "#op"\n"); } while(0)
#define R rnd() % 10

/*Pseudo-pseudorandom number generation*/
const unsigned long rmax[2] = {65267,65147};
unsigned long rstate[2] = {33333,55555};
unsigned long rfac[2] = {33333,55555};
void setseed(unsigned long s);
unsigned long rnd();

const unsigned long nSeeds = 20;
unsigned int seeds[nSeeds];
unsigned long sIndx;

unsigned int nextSeed();
void initSeeds(unsigned int ms);
void normalizeLines(std::string& s);
int compareOutput(); //parses output & soutput; reports differences.
void randString(std::string& s,unsigned long len);

/* XXX: need to explicitly check on the hash function,
 * which requires you make it public... */
int main(void)
{
	std::ofstream fout("output");
	initSeeds(0xf662ae43);

	/* first, test the hash function. */
	uint32_t A1[32];
	for (size_t i = 0; i < 32; i++) {
		A1[i] = 2*i+1;
	}
	/* make second hash trivial: */
	uint64_t ha = 1;
	uint64_t hb = 0;
	hashTbl::hash h0(64,A1,&ha,&hb);
	string x;
	dbgprint("test 0: super-simple hashing\n");
	x = "lololol.";
	fout << h0(x) << " ";
	x = "hahahaha";
	fout << h0(x) << " ";
	x = "00000000";
	fout << h0(x) << "@\n";
	dbgprint("test 1: simple hashing\n");
	x = "lololololol";
	fout << h0(x) << " ";
	x = "hahahahahaha";
	fout << h0(x) << " ";
	x = "000000000000";
	fout << h0(x) << "@\n";
	/* now make hash parameters more interesting */
	for (size_t i = 0; i < 32; i++) {
		A1[i] = 17432576*i+1;
	}
	ha = 643392968737;
	hb = 1046035321009;
	hashTbl::hash h1(64,A1,&ha,&hb);
	dbgprint("test 2: more hashing\n");
	x = "lololol.";
	fout << h1(x) << " ";
	x = "hahahaha";
	fout << h1(x) << " ";
	x = "00000000";
	fout << h1(x) << "@\n";
	dbgprint("test 3: more hashing\n");
	x = "lololololol";
	fout << h1(x) << " ";
	x = "hahahahahaha";
	fout << h1(x) << " ";
	x = "000000000000";
	fout << h1(x) << "@\n";

	/* now make some random strings and hash them with a few functions. */
	dbgprint("test 4 & 5: hashing\n");
	for (size_t i = 0; i < 10; i++) {
		randString(x, rnd() % 120 + 1);
		fout << h1(x) << ((i%5 == 4)? "@\n" : " ");
	}

	/* now test the hash table construction:
	 * hashTbl(const hashTbl& H)
     * operator=
     * insert
     * remove
     * search
     * countelements
	 * longestListLength
	 * */

	hashTbl H(5,A1,&ha,&hb);
	hashTbl HSm(3,A1,&ha,&hb);
	unsigned long i;

	const unsigned long nElts = 100;
	const unsigned long nEltsSm = 10;
	string rStrIn[nElts];
	for(i=0; i<nElts; i++)
		randString(rStrIn[i],rnd()%10+2);
	for(i=0; i<nEltsSm; i++)
		HSm.insert(rStrIn[rnd() % nElts]);

	dbgprint("test 6: copy constructor\n");
	hashTbl* hprime = new hashTbl(HSm);
	fout << *hprime << " @";
	delete hprime;

	dbgprint("test 7: operator=\n");
	hprime = new hashTbl(4);
	*hprime = HSm;
	fout << *hprime << " @";
	delete hprime;

	dbgprint("test 8: insert\n");
	for(i=0; i<45; i++)
		H.insert(rStrIn[rnd()%nElts]);
	fout << H << " @";

	dbgprint("test 9: remove\n");
	for(i=0; i<20; i++)
		H.remove(rStrIn[rnd() % nElts]);
	fout << H << " @";

	dbgprint("test 10: search\n");
	for(i=0; i<50; i++)
	{
		if(H.search(rStrIn[rnd()%nElts]))
			fout << "1 ";
		else
			fout << "0 ";
	}
	fout << " @";

	dbgprint("test 11: count elements\n");
	H.clear();
	for(i=0; i<45; i++)
		H.insert(rStrIn[rnd()%nElts]);
	for(i=0; i<30; i++) {
		fout << H.countElements() << " ";
		H.remove(rStrIn[rnd() % nElts]);
	}
	fout << " @";

	dbgprint("test 12: longest list\n");
	H.clear();
	for(i=0; i<45; i++)
		H.insert(rStrIn[rnd()%nElts]);
	for(i=0; i<20; i++) {
		fout << H.longestListLength() << " ";
		H.remove(rStrIn[rnd() % nElts]);
	}
	fout << " @\n";

	dbgprint("test 13: input output; random hash function\n");
	hashTbl H1(5);
	set<string> S1;
	size_t k;
	for (size_t i = 0; i < 75; i++) {
		k = rnd() % nElts;
		H1.insert(rStrIn[k]);
		S1.insert(rStrIn[k]);
	}
	for (size_t i = 0; i < 25; i++) {
		k = rnd() % nElts;
		H1.remove(rStrIn[k]);
		S1.erase(rStrIn[k]);
	}
	for (size_t i = 0; i < 20; i++) {
		k = rnd() % nElts;
		fout << (H1.search(rStrIn[k]))
			 << (S1.find(rStrIn[k]) != S1.end()) << " ";
	}
	fout << "@";

	fout.close();
	compareOutput();
	return 0;
}

unsigned int nextSeed()
{
	return seeds[sIndx = (sIndx+1)%nSeeds];
}

void initSeeds(unsigned int ms)
{
	sIndx = 0;
	setseed(ms);
	for(unsigned long i=0; i<nSeeds; i++)
		seeds[i] = rnd();
}
int compareOutput()
{
	char* sout, *out;
	FILE* fsout = fopen("soutput","r");
	if(fsout == 0) {
		cout << "File \"soutput\" not found to test against.\n\n";
		return -1;
	}
	FILE* fout = fopen("output","r");
	if (fout == 0) {
		cout << "File \"\" not found.\n\n";
		return -2;
	}
	//else, both files exist.  Read them.
	unsigned long ssize,osize;
	fseek(fsout,0,SEEK_END);
	ssize = ftell(fsout);
	fseek(fsout,0,SEEK_SET);
	fseek(fout,0,SEEK_END);
	osize = ftell(fout);
	fseek(fout,0,SEEK_SET);
	sout = new char[ssize+1];
	out = new char[osize+1];
	fread(sout,1,ssize,fsout);
	fread(out,1,osize,fout);
	fclose(fsout);
	fclose(fout);
	//make sure they are c-strings:
	sout[ssize] = 0;
	out[osize] = 0;
	//now tokenize and compare.	

	char* token; //the current token
	std::string answers[256]; //holds the answers. upper bound the answers by 256 for now.
	std::string results[256]; //holds the submitted answers
	char delims[] = "@"; //just the @ sign for now
	token = ::strtok(sout,delims);
	unsigned long nAnswers=0,j=0;
	while(token)
	{
		answers[nAnswers] = token;
		normalizeLines(answers[nAnswers++]);
		token = ::strtok(0,delims);
	}
	token = ::strtok(out,delims);
	while(token && j<nAnswers)
	{
		results[j] = token;
		normalizeLines(results[j++]);
		token = ::strtok(0,delims);
	}
	//now compare answers vs. results and print report.
	unsigned long nRight=0;
	for (j = 0; j < nAnswers; j++) {
		if(answers[j] == results[j])
			nRight++;
		else
			cout << "\ntest " << j << " failed:\n" <<
				"correct output:\t" << answers[j].c_str() << endl <<
				"your output:\t" << results[j].c_str() << "\n\n";
	}
	cout << "\n**********Results**********\n\n" << nRight << " out of " <<
		nAnswers << " were correct.\n\n***************************\n\n";

	delete[] sout;
	delete[] out;
	return 0;
}
void setseed(unsigned long s)
{
	rstate[0] = ((s&0xFFFF)*33333) % rmax[0];
	rstate[1] = (((s>>16)^(s&0xFFFF))*55555) % rmax[1];
	for (unsigned long i = 0; i < 2; i++) {
		if(rstate[i]%2 == 0) ++rstate[i];
		rfac[i] = rstate[i];
	}
}
unsigned long rnd()
{
	for (unsigned long i = 0; i < 2; i++) {
		rstate[i] = (rstate[i]*rfac[i])%rmax[i];
	}
	return rstate[0] | (rstate[1] << 16);
}
void normalizeLines(std::string& s)
{
	size_t pos = 0;
	std::string creturn = "\r";
	//erase all the carriage returns.
	while ((pos = s.find(creturn,pos)) != std::string::npos) {
		s.erase(pos,creturn.length());
	}
}

void randString(std::string& s,unsigned long len)
{
	unsigned long i;
	char* A = new char[len+1];
	A[len] = 0;
	for(i=0; i<len; i++)
		A[i] = (rnd() % ('z'-'A'+1)) + 'A';
	s = A;
}

