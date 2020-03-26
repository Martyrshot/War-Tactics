#include <string>
#include <vector>
#include <iostream>
#include "include/cards.hpp"
using namespace std;

// etherium address
string address;

uint8_t board[3][10][9];

vector<int> handIDs;

int main(int argc, char **argv) {
    vector<int> v;
    for (int i = 0; i < 52; i++) {
        if (i % 9 == 2) {
            cout << i << endl;
            v.push_back(i);
        } 
    }

    vector< vector<string> > hand = idsToASCII(v);
    printHand(hand);
    return 0;
}