/* Logan Seaburg
 *
 * In order to make this program I used lecture notes, reference sheets on cs106b.stanford.edu,
 * and Stanford libraries. I added pqueue.h and filelib.h.
 */

#include "encoding.h"
#include "filelib.h"
#include "pqueue.h"

Map<int, string> buildEncodingMap(HuffmanNode* encodingTree, string chosen);

/* Will return a map of characters as keys the number of times
 * that character appears in the file as values
 */
Map<int, int> buildFrequencyTable(istream& input) {
    Map<int, int> freqTable;
    char ch;
    while (input.get(ch)) {
        freqTable[ch]++;
    }
    freqTable[PSEUDO_EOF]++;
    return freqTable;
}

/* Will take the characters with their appearances and create a tree
 * full of characters with the more common ones appearing on the top
 */
HuffmanNode* buildEncodingTree(const Map<int, int>& freqTable) {
    PriorityQueue<HuffmanNode*> pq;
    for(int key : freqTable) {
        pq.add(new HuffmanNode(key, freqTable[key]), freqTable[key]);
    }
    while(pq.size() != 1) {
        HuffmanNode* temp = new HuffmanNode();
        int priority = pq.peekPriority();
        temp->zero = pq.dequeue();
        priority += pq.peekPriority();
        temp->one = pq.dequeue();
        temp->count = priority;
        pq.enqueue(temp, priority);
    }
    return pq.dequeue();
}

/* Is a function that will just call another function with an extra
 * parameter to keep track of chosen paths.
 */
Map<int, string> buildEncodingMap(HuffmanNode* encodingTree) {
    return buildEncodingMap(encodingTree, "");
}

/* Will convert the tree of characters into a map with the characters as keys
 * and the path to the tree composed of 1's and 0's as values
 */
Map<int, string> buildEncodingMap(HuffmanNode* encodingTree, string chosen) {
    Map<int, string> encodingMap;
    if (encodingTree->zero != NULL) {
        encodingMap += buildEncodingMap(encodingTree->zero, chosen + "0");
    }
    if (encodingTree->character != NOT_A_CHAR) {
        encodingMap.add(encodingTree->character, chosen);
    }
    if (encodingTree->one != NULL) {
        encodingMap += buildEncodingMap(encodingTree->one, chosen + "1");
    }
    return encodingMap;
}

/* Will loop through all of the character in the file and replace them with the path to
 * character with bits so it will take up less space than the character
 */
void encodeData(istream& input, const Map<int, string>& encodingMap, obitstream& output) {
    char ch;
    while(input.get(ch)) {
        for(char charBit : encodingMap[ch]) {
            output.writeBit(charBit - 48);
        }
    }
    for(char charBit : encodingMap[PSEUDO_EOF]) {
        output.writeBit(charBit - 48);
    }
}

/* Will read the path to the character in bits and output the actual character so
 * that it can be understood and make sense to the user/
 */
void decodeData(ibitstream& input, HuffmanNode* encodingTree, ostream& output) {
    HuffmanNode* temp = encodingTree;
    int bit;
    while (temp->character != PSEUDO_EOF) {
        bit = input.readBit();
        temp = (bit == 1) ? temp->one : temp->zero;
        if (temp->character != NOT_A_CHAR && temp->character != PSEUDO_EOF) {
          output.put(temp->character);
          temp = encodingTree;
        }
    }
}

/* Combines all of the map generation functions into one so they can be easier
 * for the user to use.
 */
void compress(istream& input, obitstream& output) {
    Map<int, int> frequencyTable = buildFrequencyTable(input);
    HuffmanNode* encodingTree =  buildEncodingTree(frequencyTable);
    Map<int, string> encodingMap = buildEncodingMap(encodingTree);
    rewindStream(input);
    output << frequencyTable;
    encodeData(input, encodingMap, output);
}

/* Reads in the map initially and then gives the actual message part to the decode function
 * so that it will decode the file with only one function.
 */
void decompress(ibitstream& input, ostream& output) {
     Map<int, int> frequencyTable;
     input >> frequencyTable;
     HuffmanNode* encodingTree =  buildEncodingTree(frequencyTable);
     decodeData(input, encodingTree, output);
}

/* Loops through all of the nodes of the tree and deletes the node
 * to free up the memory of the program.
 */
void freeTree(HuffmanNode* node) {
        if (node->zero != NULL) {
            freeTree(node->zero);
        } else if (node->one != NULL) {
            freeTree(node->one);
        }
        delete node;
}
