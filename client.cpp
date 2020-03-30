#include <string>
#include <vector>
#include <iostream>
#include "include/cards.hpp"
#include "include/userInput.hpp"
#include "include/output.hpp"
#include <stdlib.h>
#include "include/game_interface.hpp"
#include "include/prompts.hpp"
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




game_interface::GameInterface interface;

void testDriver(void);

bool createGame(void);

bool joinGame(string address);

void playGame(void);

int main(int argc, char **argv) {
    int8_t selection = -1;

    (void) argc;
    (void) argv;

    do {
        selection = mainMenu(TITLE);
    } while (selection == -1);
    if (selection == 0) {
        if (!createGame()) {
            return -1;
        }
    }
    else {
        string addr = promptAddress();
        if (!joinGame(addr)) {
            return -1;
        }
    }
    playGame();
    //testDriver();
    return 0;
}

bool createGame(void) {
    string address = interface.createGame();
    cout << "Game Address: " << address << endl;
    interface.waitPlayerJoined();
    if (!interface.hasPlayers()) {
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
    while(!interface.hasDeck())
    {
        sleep(1);
    }
    interface.waitDecksReady();
    if (!interface.drawHand()) {
        cout << "Error drawing initial hand!" << endl;
        return false;
    }
    playerID = 1;
    return true;
}

bool joinGame(string address) {
    try {
        if (!interface.joinGame(address)) {
            cout << "Failed to join game with address: " << address << endl;
            return false;
        }
    } catch(ResourceRequestFailedException const& e) {
        cout << "Exception: Failed to join game with address: " << address << endl;
        return false;
    }
    srand(time(NULL));
    uint8_t deckSeed[DECK_SIZE];
    for (int i = 0; i < DECK_SIZE; i++) {
        deckSeed[i] = rand();
    }
    if (!interface.createDeck(deckSeed)) {
        cout << "Error creating deck!" << endl;
        return false;
    }
    interface.waitDecksReady();
    if (!interface.drawHand()) {
        cout << "Error drawing initial hand!" << endl;
        return false;
    }
    playerID = 2;
    return true;
}

vector<uint8_t> buildHand(vector<uint8_t>);

void playGame(void) {
    bool hqPlaced = false;
    vector< vector<uint8_t> >points;
    vector<uint8_t> healths;

    do {
        system("clear");
        points = getPossibleHQLocations(playerID);
        healths = interface.getHqHealth();
        board = interface.getBoardState();
        printBoard(board, playerID, points, healths[0], healths[1]);
        points.clear();
        vector<uint8_t> point;
        do {
            point = promptForPoint(PROMPTHQPLACE);
        } while (point.size() == 0);

        // we only need the x coord, but ask for both for consistency
        hqPlaced = interface.placeHq(point[0]);
    } while (!hqPlaced);

    interface.waitGameStart();
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

        points.clear();
        healths = interface.getHqHealth();
        // TODO confirm which health is which
        printBoard(board, playerID, points, healths[0], healths[1]);
        printHand(handIDs);
        promptForEnter(PRPOMPTSTARTTURN);
        // clear screen, and prompt for action
        system("clear");
        printOpponentsHand(oppHandSize);
        // printBoard requires points. give it no points to highlight

        healths = interface.getHqHealth();
        // TODO confirm which health is which
        printBoard(board, playerID, points, healths[0], healths[1]);
        printHand(handIDs);
        int action = promptForAction(PROMPTACTION);
        switch (action) {
            case 1:
                {
                // lay a path
                int8_t cardID = -1;
                int handSize = handIDs.size();
                do {
                    cardID = promptForCard(PROMPTHANDSELECTION, handSize);
                } while (cardID == -1);
                vector< vector<uint8_t> > points =
                                    getAllPathPlacementOptions(board,playerID);
                system("clear");
                printOpponentsHand(oppHandSize);
                healths = interface.getHqHealth();
                // TODO confirm which health is which
                printBoard(board, playerID, points, healths[0], healths[1]);
                printHand(handIDs, cardID);
                vector<uint8_t> point;
                do {
                    point = promptForPoint(PROMPTBOARDSELECTION);
                } while (point.size() == 0);
                uint8_t adjx = 63; //error val
                uint8_t adjy = 63; //error val
                for(vector<uint8_t> tile:
                                        getAllTilesInControl(board, playerID)) {
                    for(vector<uint8_t> adjtile:
                                               getAdjacentTiles(point)) {
                        if (tile[0] == adjtile[0] && tile[1] == adjtile[1]) {
                            adjx = tile[0];
                            adjy = tile[0];
                        }
                    }
                }
                if (interface.layPath(point[0], point[1], cardID, adjx, adjy)) {
                    cout << "Failed to lay a path there!" << endl;
                }
            }
            break;
            case 2:
            {
                // Place a new unit
                int cardID = -1;
                int handSize = handIDs.size();
                do {
                    cardID = promptForCard(PROMPTHANDSELECTION, handSize);
                } while (cardID == -1);
                points.clear();
                interface.layUnit(cardID);
            }
            break;
            case 3:
            {
                // Move a unit
                vector<uint8_t> dest;
                vector<uint8_t> source;
                do {
                    source = promptForPoint(PROMPTBOARDSELECTION);
                } while (source.size() == 0);
                points =
                       getPossibleMovementOptionsForUnit(board, source);
                system("clear");
                printOpponentsHand(oppHandSize);
                healths = interface.getHqHealth();
                // TODO confirm which health is which
                printBoard(board, playerID, points, healths[0], healths[1]);
                printHand(handIDs);
                do {
                    dest = promptForPoint(PROMPTSECONDARYBOARDSELECTION);
                } while (dest.size() == 0);
                interface.moveUnit(source[0], source[1], dest[0], dest[1]);
            }
            break;
            case 4:
            {
                // Attack
                vector<uint8_t> dest;
                vector<uint8_t> source;
                do {
                    source = promptForPoint(PROMPTBOARDSELECTION);
                } while (source.size() == 0);
                points =
                       getPossibleAttackOptionsForUnit(board, playerID, source);
                system("clear");
                printOpponentsHand(oppHandSize);
                healths = interface.getHqHealth();
                // TODO confirm which health is which
                printBoard(board, playerID, points, healths[0], healths[1]);
                printHand(handIDs);
                do {
                    dest = promptForPoint(PROMPTSECONDARYBOARDSELECTION);
                } while (dest.size() == 0);
                interface.attack(source[0], source[1], dest[0], dest[1]);
            }
            break;
            default:
                //error
                cout << "Error!" << endl;
        }
        points.clear();

        system("clear");
        board = interface.getBoardState();
        printOpponentsHand(oppHandSize);
        healths = interface.getHqHealth();
        // TODO confirm which health is which
        printBoard(board, playerID, points, healths[0], healths[1]);
        printHand(handIDs);
    }
    //TODO
    // Having a nicer printout would be ideal, but for now this is fine
    if (interface.isGameOver() == playerID) {
        cout << "You win!" << endl;
    }
    else {
        cout << "You lost!" << endl;
    }
    interface.endGame();
}

vector<uint8_t> buildHand(vector<uint8_t> handSeeds) {
    vector<uint8_t> result;
    for (unsigned long i = 0; i < handSeeds.size(); i++) {
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
    vector<uint8_t> healths = {1,2};
    printBoard(board, 1, points, healths[0], healths[1]);
    printHand(v);
    printOpponentsHand(numCardsInHand);
    healths = {2,1};
    printBoard(board, 2, points, healths[0], healths[1]);
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


