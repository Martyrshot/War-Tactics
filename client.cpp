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

#define BOARDHANDSEPARATER "========================================="

void printHorizontalBar(int row, vector< vector<int> > points) {
    int curTile = -1;
    bool oneMore = false;
    for (int i = 0; i < 41; i++) {
        if (i % 4 == 0) {
            // this is going to be rank, but I don't know of a better
            // way to do it right now
            curTile++;
            bool foundTile = false;
            for (int j = 0; j < points.size(); j++) {
                if (points[j][1] == row && points[j][0] == curTile) {
                    foundTile = true;
                    oneMore = true;
                    cout << "\033[1;33m";
                }
                else if (row != 0 && points[j][1] == row - 1 &&
                            points[j][0] == curTile) {
                    foundTile = true;
                    oneMore = true;
                    cout << "\033[1;33m";
                }
            }
            cout << "+";
            if (oneMore && !foundTile) {
                cout << "\033[1;0m";
                oneMore = false;
            }
        }
        else {
            cout << "-";
        }
    }
    cout << endl;
}


void printBoard(vector< vector<int> > points) {
    cout << BOARDHANDSEPARATER << endl;
    printHorizontalBar(0,points);
    for (int i = 0; i < 9; i++) {
        bool oneMore = false;
        for (int j = 0; j < 10; j++) {
            // check state first
            bool foundSquare = false;
            for (int k = 0; k < points.size(); k++) {
                if (points[k][0] == j && points[k][1] == i) {
                    foundSquare = true;
                    oneMore = true;
                    cout << "\033[1;33m";
                }
            }
            cout << "|";
            if (oneMore && !foundSquare) {
                cout << "\033[1;0m";
                oneMore = false;
            }
            switch (board[1][j][i]){
                case (STATE_BLANK):
                    cout << "   ";
                break;
                case (STATE_PATH):
                    if (foundSquare) {
                        cout << "\033[1;0m" << " = " << "\033[1;33m";
                    }
                    else {
                        cout << " = ";
                    }
                break;
                case (STATE_HQ):
                    if (playerID == board[2][j][i]) {
                        cout << "\033[1;32m";
                    }
                    else {
                        cout << "\033[1;31m";
                    }
                    cout << " H ";
                    if (oneMore) {
                        cout << "\033[1;33m";
                    }
                    else {
                        cout << "\033[0m";
                    }
                break;
                // the last two states will both show the unit on top
                // so we can use the same case for either
                default:
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
                    else if (curcard == 1) {
                        cout << " A ";
                    }
                    else {
                        cout << " " << (int)curcard << " ";
                    }
                    if (oneMore) {
                        cout << "\033[1;33m";
                    }
                    else {
                        cout << "\033[0m";
                    }
            }
        }
        cout << "| " << "\033[0m" << i + 1 << endl;
        printHorizontalBar(i+1, points);
    }
    cout << "\033[0m";
    for (int i = 0; i < 10; i++) {
        cout << "  " << (char)(i + 65) << " ";
    }
    cout << endl << BOARDHANDSEPARATER << endl;
}

vector< vector<int> >getAllUnitsPoints(void) {
    vector< vector<int> > points;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 10; j++) {
            if ((board[1][j][i] == STATE_PATH_AND_UNIT
                            || board[1][j][i] == STATE_HQ_AND_UNIT)
                            && board[2][j][i] == playerID) {
                
                points.push_back({j, i});
            }
        }
    }
    return points;
}

vector< vector<int> > getPossibleMovementOptionsForUnit(vector<int> unit) {
    int x = unit[0];
    int y = unit[1];
    vector< vector<int> > points = { {x-1, y}, {x, y-1}, {x+1, y}, {x, y+1}};
    return points;
}

vector< vector<int> > getPossibleAttackOptionsForUnit(vector<int> unit) {
    int x = unit[0];
    int y = unit[1];
    vector< vector<int> > points;
    if (x < 9 && (board[1][x+1][y] == STATE_HQ_AND_UNIT
            || board[1][x+1][y] == STATE_PATH_AND_UNIT
            || board[1][x+1][y] == STATE_HQ)
            && board[2][x+1][y] != playerID) {
                points.push_back({x+1,y});
    }
    if (x > 0 && (board[1][x-1][y] == STATE_HQ_AND_UNIT
            || board[1][x-1][y] == STATE_PATH_AND_UNIT
            || board[1][x+1][y] == STATE_HQ)
            && board[2][x-1][y] != playerID) {
                points.push_back({x-1,y});
    }
    if (y > 0 && (board[1][x][y-1] == STATE_HQ_AND_UNIT
            || board[1][x][y-1] == STATE_PATH_AND_UNIT
            || board[1][x+1][y] == STATE_HQ)
            && board[2][x][y-1] != playerID) {
                points.push_back({x,y-1});
    }
    if (y < 8 && (board[1][x][y+1] == STATE_HQ_AND_UNIT
            || board[1][x][y+1] == STATE_PATH_AND_UNIT
            || board[1][x+1][y] == STATE_HQ)
            && board[2][x][y+1] != playerID) {
                points.push_back({x,y+1});
    }
    return points;
}

vector< vector<int> > getFriendlyEmptyHQ(void) {
    vector <vector<int> > points;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 10; j++) {
            if (board[1][j][i] == STATE_HQ
                && board[2][j][i] == playerID) {
                points.push_back({j,i});
            }
        }
    }
    return points;
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

    //vector< vector<int> > testpoints ={{0,0}, {8,8}, {4,4}, {4,3}, {4,5}, {9,8}, {8,7}};
    //vector< vector<int> > units = getAllUnitsPoints();
    //vector<int> unit = {3,3};
    //vector< vector<int> > points = getPossibleAttackOptionsForUnit(unit);
    vector< vector<int> > points = getFriendlyEmptyHQ();
    printOpponentsHand(5);
    printBoard(points);
    printHand(v);
    return 0;

}