#include <string>
#include <vector>
#include <iostream>

using namespace std;

const uint8_t STATE_BLANK = 0;
const uint8_t STATE_PATH = 1;
const uint8_t STATE_PATH_AND_UNIT = 2;
const uint8_t STATE_HQ = 3;
const uint8_t STATE_HQ_AND_UNIT = 4;

#define BOARDHANDSEPARATER "========================================="

void printHorizontalBar(int row, vector< vector<uint8_t> > points) {
    int curTile = -1;
    bool oneMore = false;
    for (int i = 0; i < 41; i++) {
        if (i % 4 == 0) {
            // this is going to be rank, but I don't know of a better
            // way to do it right now
            curTile++;
            bool foundTile = false;
            for (size_t j = 0; j < points.size(); j++) {
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


void printBoard(vector< vector< vector<uint8_t> > >board, int playerID,
                                            vector< vector<uint8_t> > points) {
    cout << BOARDHANDSEPARATER << endl;
    printHorizontalBar(0,points);
    for (int i = 0; i < 9; i++) {
        bool oneMore = false;
        for (int j = 0; j < 10; j++) {
            // check state first
            bool foundSquare = false;
            for (size_t k = 0; k < points.size(); k++) {
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

vector< vector<uint8_t> >
getAllUnitPoints(vector< vector< vector<uint8_t> > >board, int playerID) {
    vector< vector<uint8_t> > points;
    for (uint8_t i = 0; i < 9; i++) {
        for (uint8_t j = 0; j < 10; j++) {
            if ((board[1][j][i] == STATE_PATH_AND_UNIT
                            || board[1][j][i] == STATE_HQ_AND_UNIT)
                            && board[2][j][i] == playerID) {
                
                points.push_back({j, i});
            }
        }
    }
    return points;
}

vector< vector<uint8_t> >
getPossibleMovementOptionsForUnit(vector<uint8_t> unit) {
    uint8_t x = unit[0];
    uint8_t y = unit[1];
    vector< vector<uint8_t> > points = { {static_cast<unsigned char>(x-1), y},
                                        {x, static_cast<unsigned char>(y-1)}, 
                                        {static_cast<unsigned char>(x+1), y}, 
                                        {x, static_cast<unsigned char>(y+1)}};
    return points;
}


vector< vector<uint8_t> >
getAllPathPlacementOptions(vector< vector< vector<uint8_t> > >board,
                                                                int playerID) {
    vector< vector<uint8_t> > points;
    vector< vector<uint8_t> > units = getAllUnitPoints(board, playerID);

    for (vector<uint8_t> unit: units) {

        uint8_t x = unit[0];
        uint8_t y = unit[1];

        if (x < 9 && board[1][x+1][y] == STATE_BLANK) {
                points.push_back({static_cast<unsigned char>(x+1),y});
        }

        if (x > 0 && board[1][x-1][y] == STATE_BLANK) {
                    points.push_back({static_cast<unsigned char>(x-1),y});
        }
        if (y > 0 && board[1][x][y-1] == STATE_BLANK) {
                    points.push_back({x,static_cast<unsigned char>(y-1)});
        }
        if (y < 8 && board[1][x][y+1] == STATE_BLANK) {
                    points.push_back({x,static_cast<unsigned char>(y+1)});
        }
    }
    cout << "here!--" << endl;
    return points;
}

vector< vector<uint8_t> >
getPossibleAttackOptionsForUnit(vector< vector< vector<uint8_t> > >board,
                                    int playerID, vector<int> unit) {
    uint8_t x = unit[0];
    uint8_t y = unit[1];
    vector< vector<uint8_t> > points;
    if (x < 9 && (board[1][x+1][y] == STATE_HQ_AND_UNIT
            || board[1][x+1][y] == STATE_PATH_AND_UNIT
            || board[1][x+1][y] == STATE_HQ)
            && board[2][x+1][y] != playerID) {
                points.push_back({static_cast<unsigned char>(x+1),y});
    }
    if (x > 0 && (board[1][x-1][y] == STATE_HQ_AND_UNIT
            || board[1][x-1][y] == STATE_PATH_AND_UNIT
            || board[1][x-1][y] == STATE_HQ)
            && board[2][x-1][y] != playerID) {
                points.push_back({static_cast<unsigned char>(x-1),y});
    }
    if (y > 0 && (board[1][x][y-1] == STATE_HQ_AND_UNIT
            || board[1][x][y-1] == STATE_PATH_AND_UNIT
            || board[1][x][y-1] == STATE_HQ)
            && board[2][x][y-1] != playerID) {
                points.push_back({x,static_cast<unsigned char>(y-1)});
    }
    if (y < 8 && (board[1][x][y+1] == STATE_HQ_AND_UNIT
            || board[1][x][y+1] == STATE_PATH_AND_UNIT
            || board[1][x][y+1] == STATE_HQ)
            && board[2][x][y+1] != playerID) {
                points.push_back({x,static_cast<unsigned char>(y+1)});
    }
    return points;
}

vector< vector<uint8_t> >
getFriendlyEmptyHQ(vector< vector< vector<uint8_t> > >board, int playerID) {
    vector <vector<uint8_t> > points;
    for (uint8_t i = 0; i < 9; i++) {
        for (uint8_t j = 0; j < 10; j++) {
            if (board[1][j][i] == STATE_HQ
                && board[2][j][i] == playerID) {
                points.push_back({j,i});
            }
        }
    }
    return points;
}
