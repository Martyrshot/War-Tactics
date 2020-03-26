#include <string>
#include <vector>
#include <iostream>
#include "include/cards.hpp"
#include <stdlib.h>
#include <time.h> // TODO remove this after testing is complete
using namespace std;

// etherium address
string address;

// store the current board so we don't have to keep asking for it
uint8_t board[3][10][9];

// current hand state
vector<int> handIDs;

	const uint8_t STATE_BLANK = 0;
	const uint8_t STATE_PATH = 1;
	const uint8_t STATE_PATH_AND_UNIT = 2;
	const uint8_t STATE_HQ = 3;
	const uint8_t STATE_HQ_AND_UNIT = 4;

int playerID;

#define BOARDHORIZONTALBAR "+---+---+---+---+---+---+---+---+---+---+"
#define BOARDHANDSEPARATER "========================================="
void printBoard(void) {
    cout << BOARDHANDSEPARATER << endl;
    cout << BOARDHORIZONTALBAR << endl;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 10; j++) {
            // check state first
            switch (board[1][j][i]){
                case (STATE_BLANK):
                    cout << "|   ";
                break;
                case (STATE_PATH):
                    cout << "| = ";
                break;
                case (STATE_HQ):
                    cout << "|";
                    if (playerID == board[2][j][i]) {
                        cout << "\033[1;32m";
                    }
                    else {
                        cout << "\033[1;31m";
                    }
                    cout << " H ";
                    cout << "\033[0m";
                break;
                // the last two states will both show the unit on top
                // so we can use the same case for either
                default:
                    cout << "|";
                    if (playerID == board[2][j][i]) {
                        cout << "\033[1;32m";
                    }
                    else {
                        cout << "\033[1;31m";
                    }
                    int curcard = board[0][i][j] % 13;
                    if (curcard == 10) {
                        cout << " " << (int)curcard;
                    }
                    else if (curcard == 11) {
                        cout << " J ";
                    }
                    else if (curcard == 12) {
                        cout << " Q " ;
                    }
                    else if (curcard == 13) {
                        cout << " K ";
                    }
                    else {
                        cout << " " << (int)curcard << " ";
                    }
                    cout << "\033[0m";
            }
        }
        cout << "| " << i + 1 << endl << BOARDHORIZONTALBAR << endl;
    }
    for (int i = 0; i < 10; i++) {
        cout << "  " << (char)(i + 65) << " ";
    }
    cout << endl << BOARDHANDSEPARATER << endl;
}


int main(int argc, char **argv) {
    vector<int> v;
    for (int i = 0; i < 52; i++) {
        if (i % 10 == 2) {
            v.push_back(i);
        } 
    }
    // Dumby initialization
    playerID = 2;
    srand(time(NULL));
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 9; j++) {
            board[0][i][j] = rand() % 52;
            board[1][i][j] = rand() % 5;
            board[2][i][j] = rand() % 2 + 1;
        }
    }

    printOpponentsHand(5);
    printBoard();
    printHand(v);
    return 0;

}