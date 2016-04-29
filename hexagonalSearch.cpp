/*
 * File: hexagonalSearch.cpp
 * Author: Robin Zhang
 * -------------------------
 * This program was written for a coding challenge.
 * Given a honeycomb tessellation of hexagonal cells that
 * each contain a single letter and a dictionary of words,
 * this program returns the list of words contained
 * in the honeycomb. Words are formed by paths through
 * adjacent cells.
 *
 * General idea: Keep a linked list of nodes that have
 * the value of the same letter. For each word in the
 * dictionary, iterate through the linked list of nodes
 * and perform a depth-first search through each node's
 * neighbor looking for the next letter of the word.
 *
 * DFS chosen over BFS since depth is at most the length
 * of the longest word in the dictionary which is unlikely
 * to be very long. Memory usage is much lower as a result.
 */

/* Packages */
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

/* Namespace */
using namespace std;

/* Macros */
#define SIDES 6
#define ALPHABET 26

/*
 * Struct defining a node in a polygonal structure
 * of characters. The nodes contain a value, pointers
 * to each of its adjacent nodes, and a next pointer
 * for use in linked lists.
 * The null adjacent pointers, if any exist, are not
 * necessarily at the end of the array.
 * For hexagonal polygons, the adjacents should come
 * in the predetermined order of lowest layer index to
 * greatest and lowest sublayer index to greatest.
 */
template<size_t adjacentN = SIDES>
struct linkedPolygonNode
{
	/* Data */
	char value = 0;
	size_t layerN = 0;
	size_t charN = 0;
	linkedPolygonNode * adjacentList[adjacentN] = { NULL };
	linkedPolygonNode * nextPtr = NULL;
	bool visited = false;

	/* Functions */
	linkedPolygonNode * getLastNode() {
		linkedPolygonNode *current = this;
		while (current->nextPtr != NULL) {
			current = current->nextPtr;
		}
		return current;
	}
};

/*
 * Function name: readLines(argn, argc, argv, layers)
 * Sets a given pointer to an array of strings to
 * point at a dynamically allocated container T of
 * lines from the input file.
 * An input gives the option of reading the first line
 * as the number of lines in the file (T must be vector<string> in this case).
 * T should be a container of strings with push_back.
 * Exits with error if file not found
 */
template<typename T>
const T readLines(int argc, char **argv, int argn, bool firstLineCount) {
	T lines;
	if (argc > argn) {
		ifstream inFile(argv[argn]);
		string line;

		if (firstLineCount) { //if first line gives number of lines
			getline(inFile, line);
			size_t lineCount = stoul(line); //convert first line to uint
			lines.reserve(lineCount);
		}

		while (getline(inFile, line)) { //until reaches end of file
			lines.push_back(line); //add line
		}

		inFile.close();
	} else exit(1);

	return lines;
}

/*
 * Function name: getBucket(value)
 * Returns the position of a given capital letter amongst all
 * capital letters.
 */
const inline size_t getBucket(const char value) {
	return value - 'A';
}

/*
 * Function name: getBucket(value)
 * Returns whether the node at the given coordinates is a corner node
 */
const inline bool isCorner(const size_t layerN, const size_t charN) {
	if (layerN == 0) return true;
	return (charN % layerN == 0);
}

/*
 * Function name: populateLinkedNodeArray(layers, linkedNodeArray, positionNodeArray)
 * Fills an array of linked lists of polygon nodes using a given
 * polygon of characters. Iterates through the layers from lowest
 * to greatest and links nodes during collisions.
 * WARNING: does not set neighbors!
 * The nodes are dynamically allocated.
 */
void populateLinkedNodeArray(const vector<string> &layers, linkedPolygonNode<SIDES> ** linkedNodeArray, vector< vector< linkedPolygonNode<SIDES> * > > &positionNodeArray) {
	for (size_t layerN = 0; layerN < layers.size(); layerN++) {
		string layer = layers[layerN];

		vector<linkedPolygonNode<SIDES> *> layerVector; //create vector of nodes for current layer
		positionNodeArray.push_back(layerVector);

		for (size_t charN = 0; charN < layer.length(); charN++) {
			char value = layer[charN];
			linkedPolygonNode<SIDES> *newNode = new linkedPolygonNode<SIDES>;

			//set values and coordinates
			newNode->value = value;
			newNode->layerN = layerN;
			newNode->charN = charN;

			//set node in position array of nodes
			(positionNodeArray[layerN]).push_back(newNode);

			//set node in array of linked lists
			size_t bucket = getBucket(value);

			if (linkedNodeArray[bucket] == NULL) linkedNodeArray[bucket] = newNode; //set first node to new node
			else {
				linkedPolygonNode<SIDES> *last = linkedNodeArray[bucket]->getLastNode();
				last->nextPtr = newNode; //set end of linked list to new node
			}
		}
	}
}

/*
 * Function name: setNeighbors(positionNodeArray)
 * Sets the adjacent node pointers of all nodes in the position node array
 * Values for neighbors' coordinates follow from mathematical derivation
 * Specific to SIDES = 6
 */
void setNeighbors(vector< vector< linkedPolygonNode<SIDES> * > > &positionNodeArray) {
	size_t layerCount = positionNodeArray.size();
	for (size_t layerN = 0; layerN < layerCount; layerN++) {
		size_t charCount = positionNodeArray[layerN].size();
		for (size_t charN = 0; charN < charCount; charN++) {
			linkedPolygonNode<SIDES> * current = positionNodeArray[layerN][charN];

			if (layerN > 0) {
				//inside neighbor (if corner) or inside right neighbor (otherwise)
				if (charN < charCount - 1) current->adjacentList[0] = positionNodeArray[layerN - 1][(layerN - 1) * (charN / layerN) + (charN % layerN)];
				else current->adjacentList[0] = positionNodeArray[layerN - 1][0];
			
				//inside left neighbor (only exists if not corner)
				if (!isCorner(layerN, charN)) {
					current->adjacentList[5] = positionNodeArray[layerN - 1][(layerN - 1) * (charN / layerN) + (charN % layerN) - 1];
				}

				//left neighbor
				if (charN > 0) current->adjacentList[1] = positionNodeArray[layerN][charN - 1];
				else current->adjacentList[1] = positionNodeArray[layerN][charCount - 1];

				//right neighbor
				if (charN < charCount - 1) current->adjacentList[2] = positionNodeArray[layerN][charN + 1];
				else current->adjacentList[2] = positionNodeArray[layerN][0];

				if (layerN < layerCount - 1) {
					//outside left neighbor (only exists if corner)
					if (isCorner(layerN, charN)) {
						if (charN > 0) current->adjacentList[5] = positionNodeArray[layerN + 1][(layerN + 1) * (charN / layerN) - 1];
						else current->adjacentList[5] = positionNodeArray[layerN + 1][charCount + SIDES - 1]; //last of next layer
					}

					//outside middle neighbor (if corner) or outside left (otherwise)
					current->adjacentList[3] = positionNodeArray[layerN + 1][(layerN + 1) * (charN / layerN) + (charN % layerN)];

					//outside right neighbor
					current->adjacentList[4] = positionNodeArray[layerN + 1][(layerN + 1) * (charN / layerN) + (charN % layerN) + 1];
				}
			} else if (layerCount > 1) {
				//manually set for central node
				for (size_t neighborN = 0; neighborN < SIDES; neighborN++) {
					current->adjacentList[neighborN] = positionNodeArray[1][neighborN];
				}
			}
		}
	}
}

/*
 * Function name: searchNodes(linkedNodeArray, word)
 * Searches an array of linked lists of polygon nodes for a given word
 * Iterates over all nodes with the first letter and then searches
 * over neighbors for subsequent letters via recursive depth-first search
 */
bool searchNodes(linkedPolygonNode<SIDES> ** linkedNodeArray, const string word, linkedPolygonNode<SIDES> * current) {
	if (current == NULL) return false; //dead-end
	if (word.length() == 0) return true; //found!

	char first = word[0];
	current->visited = true;

	for (size_t neighborN = 0; neighborN < SIDES; neighborN++) { //iterate over neighbors
		linkedPolygonNode<SIDES> * neighbor = (current->adjacentList)[neighborN];

		if (neighbor != NULL && neighbor->visited == false && neighbor->value == first) {
			if (searchNodes(linkedNodeArray, word.substr(1), neighbor)) {
				current->visited = false;
				return true; //depth-first recursion
			}
		}
	}

	//reset and back-track
	current->visited = false;
	return false;
}

/*
 * Function name: main(argc, argv)
 * Main function that searches for given words in a given honeycomb
 * Example usage: "./hexagonalSearch honeycomb.txt dictionary.txt"
 */
int main(int argc, char **argv) {
	//IO
	const vector<string> layers = readLines< vector<string> >(argc, argv, 1, true);
	const vector<string> dictionary = readLines< vector<string> >(argc, argv, 2, false);

	//initialization
	vector< vector<linkedPolygonNode<SIDES> *> > positionNodeArray; //array of vectors of nodes depicting position
	linkedPolygonNode<SIDES> * linkedNodeArray[ALPHABET] = { NULL }; //an array of linked lists of nodes (one per letter)
	populateLinkedNodeArray(layers, linkedNodeArray, positionNodeArray); //fill array with data from honeycomb
	setNeighbors(positionNodeArray);

	//iterate through words in dictionary and search
	vector<string> found;
	for (string word : dictionary) {
		char first = word[0];
		size_t bucket = getBucket(first);
		linkedPolygonNode<SIDES> * current = linkedNodeArray[bucket]; //start of linked list

		while (current != NULL) { //iterate through linked list
			if (searchNodes(linkedNodeArray, word.substr(1), current)) { //found
				found.push_back(word);
				break;
			}

			current = current->nextPtr; //advance to next starter node in linked list
		}
	}

	//use standard C++ sort and print
	sort(found.begin(), found.end());
	for (string word : found){
		cout << word << endl;
	}

	//free dynamically allocated nodes
	for (size_t layerN = 0; layerN < positionNodeArray.size(); layerN++) {
		for (size_t charN = 0; charN < positionNodeArray[layerN].size(); charN++) {
			delete(positionNodeArray[layerN][charN]);
		}
	}

	return 0;
}