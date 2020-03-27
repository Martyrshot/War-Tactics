#include <string>
#include <vector>
#include <iostream>
#include "include/cards.hpp"
#include "include/userInput.hpp"
#include "include/output.hpp"
#include <stdlib.h>
#include "include/game_interface.hpp"
#include "include/prompts.hpp"
#include <conio.h>
#include <time.h> // TODO remove this after testing is complete
using namespace std;

// etherium address
string address;

// Store the latest board so we don't have to keep requesting it
vector< vector< vector<uint8_t> > > board;

// current hand state
vector<uint8_t> handSeeds;
vector<uint8_t> handIDs;

uint8_t numCardsInHand = 0;

uint8_t playerID;

const uint8_t DECK_SIZE = 52;



GameInterface interface = new GameInterface();

void testDriver(void);

int main(int argc, char **argv) {
    testDriver();
    return 0;
}

bool createGame(void) {
    string address = interface.createGame();
    cout << "Game Address: " << address << endl;
    interface.waitPlayerJoined();
    if (!interface.hasPlayers) {
        cout << "A player joined, but we don't have players?\n" << endl;
        return false;
    }
    uint8_t deckSeed[DECK_SIZE];
    srand(time(NULL));
    for (int i = 0; i < DECK_SIZE; i++) {
        deckSeed[i] = rand();
    }
    if (!interface.createDeck(deckSeed)) {
        cout << "Error creating deck!" << endl;
        return false;
    }
    while(!interface.hasDeck());
    if (!interface.drawHand()) {
        cout << "Error drawing initial hand!" << endl;
        return false;
    }
    playerID = 1;
    return true;
}

void joinGame(string address) {
    if (!interface.joinGame(address)) {
        cout << "Failed to join game with address: " << address << endl;
        return;
    }
    srand(time(NULL));
    for (int i = 0; i < DECK_SIZE; i++) {
        deckSeed[i] = rand();
    }
    if (!interface.createDeck(deckSeed)) {
        cout << "Error creating deck!" << endl;
        return false;
    }
    while(!interface.hasDeck());
    if (!interface.drawHand()) {
        cout << "Error drawing initial hand!" << endl;
        return false;
    }
    playerID = 2;
    return true;    
}

void playGame(void) {
    while(!interface.isGameOver()) {
        interface.waitNextTurn();
        board = interface.getBoardState();
        handSeeds = interface.getPlayerSeedHand(playerID);
        handIDs = buildHand(handSeeds);
        vector<uint8_t> oppHand;
        if (playerID == 1) {
            oppHand = interface.getPlayerSeedHand(2);
        }
        else {
            oppHand = interface.getPlayerSeedHand(1);
        }
        int oppHandSize = oppHand.size();

        // Print initial turn state
        printOpponentsHand(oppHandSize);
        // printBoard requires points. give it no points to highlight
        vector< vector<uint8_t> > points;
        printBoard(board, playerID, points);
        printHand(handIDs);
        promptForEnter(PRPOMPTSTARTTURN);
        // clear screen, and prompt for action
        clrscr();
        printOpponentsHand(oppHandSize);
        // printBoard requires points. give it no points to highlight
        printBoard(board, playerID, points);
        printHand(handIDs);
        int action = promptForAction(PROMPTACTION);
        switch (action) {
            case 1:
                // lay a path
                int cardID = promptForCard(PROMPTHANDSELECTION);
                vector< vector<uint8_t> > points =
                                    getAllPathPlacementOptions(board,playerID);
                clrscr();
                printOpponentsHand(oppHandSize);
                printBoard(board, playerID, points);
                printHand(handIDs, cardID);
                vector<uint8_t> point = promptForPoint(PROMPTBOARDSELECTION);
                uint8_t adjx = 63; //error val
                uint8_t adjy = 63; //error val
                for(vector<uint8_t> tile:
                                        getAllTilesInControl(board, playerID)) {
                    for(vector<uint8_t> adjtile:
                                               getAdjacentTiles(board, point)) {
                        if (tile[0] == adjtile[0] && tile[1] == adjtile[1]) {
                            adjx = tile[0];
                            adjy = tile[0];
                        }
                    }
                }
                if (interface.layPath(point[0], point[1], cardID, adjx, adjy)) {
                    cout << "Failed to lay a path there!" << endl;
                }
            break;
            case 2:
                // Place a new unit
            break;
            case 3:
                // Move a unit
            break;
            case 4:
                //Attack
            break;
            default:
                //error
        }
    }
}

vector<uint8_t> buildHand(vector<uint8_t> handSeeds) {
    vector<uint8_t> result;
    for (int i = 0; i < handSeeds.size(); i++) {
        result.push_back(interface.getPrivateCardFromSeed(handSeeds[i]));
    }
    return result;
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

