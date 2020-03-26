#include <string>
#include <vector>
#include <iostream>
#include "include/cards.hpp"
using namespace std;

// etherium address
string address;

// store the current board so we don't have to keep asking for it
uint8_t board[3][10][9];

// current hand state
vector<int> handIDs;

#define BOARDHORIZONTALBAR "+---+---+---+---+---+---+---+---+---+---+"
#define BOARDHANDSEPARATER "========================================="
void printBoard(void) {
    cout << BOARDHANDSEPARATER << endl;
    cout << BOARDHORIZONTALBAR << endl;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 10; j++) {
            cout << "|   ";
        }
        cout << "| " << i << endl << BOARDHORIZONTALBAR << endl;
    }
    for (int i = 0; i < 10; i++) {
        cout << "  " << (char)(i + 65) << " ";
    }
    cout << endl << BOARDHANDSEPARATER << endl;
}


int main(int argc, char **argv) {
    vector<int> v;
    for (int i = 0; i < 52; i++) {
        if (i % 9 == 2) {
            v.push_back(i);
        } 
    }

    printBoard();
    printHand(v);
    return 0;
}