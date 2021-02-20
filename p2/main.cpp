/********************************************
  Test program for hash table.
*********************************************/

#include <iostream>
using std::cin;
using std::cout;
using std::endl;

#include "hashtbl.h"
using namespace csc212;

int main(void)
{
	cout << "****************Welcome to the hash table test program*****************\n\n";

	csc212::hashTbl H(4); /* small 16 element table. */
	char input;
	char inputLine[64];
	inputLine[63] = 0;
	csc212::val_type elt;

	while(true)
	{
		cout << "\nWhat would you like to do?\n(I)nsert a string | ";
		cout << "(R)emove a string | Determine if the table is (E)mpty\n";
		cout << "Find the (L)ength of the table | (C)lear the table | (P)rint table\n";
		cout << "(G)et statistics | (S)earch the table | (Q) Quit the program\n";

		cin >> input;

		input = ::tolower(input);
		switch(input)
		{
		case 'i':
			cout << "Enter an item to insert: ";
			cin.ignore(1,'\n');
			cin.getline(inputLine,63);
			elt = inputLine;
			H.insert(elt);
			break;
		case 'r':
			cout << "Enter an item to remove: ";
			cin.ignore(1,'\n');
			cin.getline(inputLine,63);
			elt = inputLine;
			H.remove(elt);
			break;
		case 'e':
			if(H.isEmpty())
				cout << "Table is empty.\n";
			else
				cout << "Table is non-empty.\n";
			break;
		case 'l':
			cout << "Table consists of " << H.tableLength() << " linked lists.\n";
			break;
		case 'c':
			H.clear();
			cout << "Table has been cleared.\n";
			break;
		case 'p':
			cout << "Table contents:\n" << H << endl;
			break;
		case 's':
			cout << "Enter string to search for: ";
			cin.ignore(1,'\n');
			cin.getline(inputLine,63);
			elt = inputLine;
			if(H.search(elt))
				cout << elt << " was found.\n";
			else
				cout << elt <<  " was not found.\n";
			break;
		case 'g':
			cout << "The list contains " << H.countElements() << " total strings, ";
			cout << H.countCollisions() << " lists with collisions,\n";
			cout << "and the longest list is of length " << H.longestListLength() << ".\n\n";
			break;
		case 'q':
			return 0;
			break;
		default:
			cout << "I'm sorry I don't understand that selction. Please choose an item from the list.\n";
		}
	}
	return 0;
}
