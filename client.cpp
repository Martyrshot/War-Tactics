#include <string>
#include <vector>
#include <iostream>
#include "include/cards.hpp"
#include "include/userInput.hpp"
#include "include/output.hpp"
#include <stdlib.h>
#include <time.h> // TODO remove this after testing is complete
using namespace std;

// etherium address
string address;

// Store the latest board so we don't have to keep requesting it
vector< vector< vector<uint8_t> > > board;

// current hand state
vector<uint8_t> handIDs;

uint8_t numCardsInHand = 0;

uint8_t playerID;

void testDriver(void);

int main(int argc, char **argv) {
    testDriver();
    return 0;
}

void testDriver(void) {
    vector<uint8_t> v;
    for (uint8_t i = 0; i < 52; i++) {
        if (i % 10 == 2) {
            v.push_back(i);
        } 
    }


    // Dumby initialization
    playerID = 2;
    srand(time(NULL));
    vector< vector<uint8_t> > r0;
    vector< vector<uint8_t> > r1;
    vector< vector<uint8_t> > r2;
    for (int i = 0; i < 10; i++) {
        vector< uint8_t> v0;
        vector< uint8_t> v1;
        vector< uint8_t> v2;
        for (int j = 0; j < 9; j++) {
            v0.push_back(rand() % 52);
            v1.push_back(rand() % 5);
            v2.push_back(rand() % 2 + 1);
        }
        r0.push_back(v0);
        r1.push_back(v1);
        r2.push_back(v2);
    }
    board.push_back(r0);
    board.push_back(r1);
    board.push_back(r2);
    //vector< vector<int> > testpoints ={{0,0}, {8,8}, {4,4}, {4,3}, {4,5}, {9,8}, {8,7}};
    //vector< vector<uint8_t> > points = getAllUnitPoints(board, playerID);
    //vector<int> unit = {3,3};
    //vector< vector<int> > points = getPossibleAttackOptionsForUnit(unit);
    //vector< vector<uint8_t> > points = getFriendlyEmptyHQ(board,playerID);
    vector< vector<uint8_t> > points = 
                                    getAllPathPlacementOptions(board, playerID);
    numCardsInHand = 5;
    printOpponentsHand(numCardsInHand);
    printBoard(board, playerID, points);
    printHand(v);
    promptForEnter("This is a test prompt\n");
    vector<uint8_t> point;
    do {
        point = promptForPoint("Please enter a point\n");
    } while(point.size() != 2);
    int input = -1;
    do {
        input = promptForAction("Enter 1,2,3 or 4\n");
    } while(input == -1);
    input = -1;
    do {
        input = promptForCard("Enter 1,2,3 4, or 5\n", 5);
    } while(input == -1);


}

