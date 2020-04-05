#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "include/cards.hpp"
#include "include/userInput.hpp"
#include "include/output.hpp"
#include <stdlib.h>
#include "include/game_interface.hpp"
#include "include/prompts.hpp"
#include <time.h> // TODO remove this after testing is complete
#ifndef DARWIN
#include <menu.h> //For ncurses
#include <ncursesw/ncurses.h>
#endif
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
uint8_t oppID;





game_interface::GameInterface interface;

void testDriver(void);

bool createGame(void);

bool joinGame(string address);

void playGame(void);

vector< vector< vector<uint8_t> > >
safeGetBoardState(vector< vector< vector<uint8_t> > >curBoard,
                                                             bool printNum);
#ifndef DARWIN
//Free the menu and remove it from the screen. Does not close any windows
//associated with the menu.
void cleanMenu(MENU *menu, ITEM **menu_items) {
	  if (NULL != menu_items) {
			for (int i = 0; i < MENU_ITEMS; i++) {
					free_item(menu_items[i]);
			}
		}
		unpost_menu(menu);
		free_menu(menu);
}	
#endif


int main(int argc, char **argv) {
#ifndef DARWIN
    (void) argc;
    (void) argv;

    unsigned int randSeed;
    ifstream irand("/dev/urandom", ios::binary);
    int selection = -1;

    irand.read((char*) &randSeed, sizeof(randSeed));
    srand(randSeed);
    irand.close();

//MENU_ITEMS+1 because new_item MUST be NULL terminated before it's used
//in new_menu. So part of the initialization is creating a NULL item....
//gf 7 hours.
		const char *menu_strings[] = {"Make a game",
			                            "Join a game",
																	"Exit",
																	NULL};

		ITEM *menu_items[MENU_ITEMS+1];
		ITEM *currentItem;
		MENU *main_menu;
		WINDOW *main_win;

//Init ncurses
//MUST include this for unicode support.
		setlocale(LC_CTYPE, "");
		initscr();
		if (has_colors()) {
 		    start_color();
		}
		cbreak();
		noecho();

		printw(TITLE);
		refresh();
		keypad(stdscr, TRUE);
    init_pair(2, COLOR_BLUE, COLOR_BLACK);
		bkgd(COLOR_PAIR(2));
    init_pair(1, COLOR_RED, COLOR_BLACK);
//Create a menu to choose a game option from
		for (int i = 0; i < MENU_ITEMS+1; i++) {
			 menu_items[i] = new_item(menu_strings[i], NULL);
		}
    main_menu = new_menu(menu_items);
		main_win = newwin(10,42,13,0);
		keypad(main_win, TRUE);
		set_menu_win(main_menu, main_win);
		set_menu_sub(main_menu, derwin(main_win, 3, 32, 3, 5));
		set_menu_mark(main_menu, " * ");
		box(main_win, 0, 0);
		wattron(main_win, COLOR_PAIR(1));
		wattron(main_win, A_BOLD);
		mvwprintw(main_win, 1, 15, "%s", "Main Menu");
// \u expects exactly 4 hex digits while \U expects exactly 8 hex digits
// Unfortunately, the command line can only display 4 byte unicode.
		mvwprintw(main_win, 2, 16, "%lc %lc %lc %lc",
				                              L'\u2660',L'\u2665',L'\u2666',L'\u2663');
		wattroff(main_win, A_BOLD);
		wattroff(main_win, COLOR_PAIR(1));
		refresh();
		post_menu(main_menu);
		wrefresh(main_win);
//There is a macro called KEY_ENTER, but the enter key value is either 10 or
//13 (NL or CR)
    while ((selection = wgetch(main_win)) != 10 && selection != 13) {
		    switch(selection) {
				    case KEY_DOWN: menu_driver(main_menu, REQ_DOWN_ITEM);
						    break;
            case KEY_UP: menu_driver(main_menu, REQ_UP_ITEM);
								break;
				}
				wrefresh(main_win);
				currentItem = current_item(main_menu);
		}

        //Decide what to do based on the menu selection
    if ((item_index(currentItem) == 0)) {
				cleanMenu(main_menu, menu_items);
				endwin();
        if (!createGame()) {
            return -1;
        }
    }
    else if (item_index(currentItem) == 1) {
				cleanMenu(main_menu, menu_items);
				endwin();
        string addr = promptAddress();
        if (!joinGame(addr)) {
            return -1;
        }
    }
	else {
			cleanMenu(main_menu, menu_items);
			endwin();
	    exit(EXIT_SUCCESS);
	}
#else
    unsigned int randSeed;
    ifstream irand("/dev/urandom", ios::binary);
    int8_t selection = -1;

    (void) argc;
    (void) argv;

    irand.read((char*) &randSeed, sizeof(randSeed));
    srand(randSeed);
    irand.close();

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
#endif

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
    playerID = 1;
    oppID = 2;
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
    oppID = 1;
    return true;
}

vector<uint8_t> buildHand(vector<uint8_t>);

void playGame(void) {
    bool hqPlaced = false;
    vector< vector<uint8_t> >points;
    vector<uint8_t> healths;


    system("clear");
    points = getPossibleHQLocations(playerID);
    healths = interface.getHqHealth();
    board = safeGetBoardState(board, true);
    printBoard(board, playerID, points, healths[playerID - 1],
                                                         healths[oppID -1]);
    points.clear();
    vector<uint8_t> point;
    do {
        point = promptForPoint(PROMPTHQPLACE);
    } while (point.size() != 2);

    // we only need the x coord, but ask for both for consistency
    hqPlaced = interface.placeHq(point[0]);

    while (!hqPlaced) {
        system("clear");
        points = getPossibleHQLocations(playerID);
        healths = interface.getHqHealth();
        printBoard(board, playerID, points, healths[playerID - 1],
                                                             healths[oppID -1]);
        points.clear();
        vector<uint8_t> point;
        do {
            point = promptForPoint(PROMPTHQPLACE);
        } while (point.size() == 0);

        // we only need the x coord, but ask for both for consistency
        hqPlaced = interface.placeHq(point[0]);

    }

    system("clear");
    points.clear();
    healths = interface.getHqHealth();
    board = safeGetBoardState(board, false);
    printBoard(board, playerID, points, healths[playerID - 1],
                                                             healths[oppID -1]);
    interface.waitGameStart();
    interface.waitNextTurn();

    while(!interface.isGameOver()) {
        system("clear");
        board = safeGetBoardState(board, false);
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
        printBoard(board, playerID, points, healths[playerID - 1],
                                                             healths[oppID -1]);
        printHand(handIDs);
        //interface.waitNextTurn();
        selectAction:
        int action = promptForAction(PROMPTACTION);
        switch (action) {
            case 0:
                {
                // lay a path
                int8_t cardID = -1;
                int handSize = handIDs.size();
                do {
                    cardID = promptForCard(PROMPTHANDSELECTION, handSize);
                    if (cardID == -2) {
                        // user wants to return to action selection
                        goto selectAction;
                    }
                } while (cardID == -1);
                vector< vector<uint8_t> > points =
                                    getAllPathPlacementOptions(board,playerID);
                system("clear");
                printOpponentsHand(oppHandSize);
                healths = interface.getHqHealth();
                // TODO confirm which health is which
                printBoard(board, playerID, points, healths[playerID - 1],
                                                             healths[oppID -1]);
                printHand(handIDs, cardID);
                vector<uint8_t> point;
                do {
                    point = promptForPoint(PROMPTBOARDSELECTION);
                    if (point.size() == 1) {
                        // user wants to return to action menu
                        point.clear();
                        goto selectAction;
                    }
                } while (point.size() != 2);
                uint8_t adjx = 63; //error val
                uint8_t adjy = 63; //error val
                for(vector<uint8_t> tile:
                                        getAllTilesInControl(board, playerID)) {
                    for(vector<uint8_t> adjtile:
                                               getAdjacentTiles(point)) {
                        if (tile[0] == adjtile[0] && tile[1] == adjtile[1]) {
                            adjx = tile[0];
                            adjy = tile[1];
                        }
                    }
                }
                if (!interface.layPath(point[0], point[1], cardID, adjx, adjy)) {
                    cout << "Failed to lay a path there!" << endl;
                }
            }
            break;
            case 1:
            {
                // Place a new unit
                int cardID = -1;
                int handSize = handIDs.size();
                do {
                    cardID = promptForCard(PROMPTHANDSELECTION, handSize);
                    if (cardID == -2) {
                        // user wants to return to action selection
                        goto selectAction;
                    }
                } while (cardID == -1);
                points.clear();
                interface.layUnit(cardID);
            }
            break;
            case 2:
            {
                // Move a unit
                vector<uint8_t> dest;
                vector<uint8_t> source;
                do {
                    source = promptForPoint(PROMPTBOARDSELECTION);
                    if (source.size() == 1) {
                        // user wants to return to action menu
                        source.clear();
                        goto selectAction;
                    }
                } while (source.size() != 2);
                points =
                    getPossibleMovementOptionsForUnit(board, source, playerID);
                system("clear");
                printOpponentsHand(oppHandSize);
                healths = interface.getHqHealth();
                // TODO confirm which health is which
                printBoard(board, playerID, points, healths[playerID - 1],
                                                             healths[oppID -1]);
                printHand(handIDs);
                do {
                    dest = promptForPoint(PROMPTSECONDARYBOARDSELECTION);
                    if (dest.size() == 1) {
                        // user wants to return to action menu
                        dest.clear();
                        goto selectAction;
                    }
                } while (dest.size() != 2);
                interface.moveUnit(source[0], source[1], dest[0], dest[1]);
            }
            break;
            case 3:
            {
                // Attack
                vector<uint8_t> dest;
                vector<uint8_t> source;
                do {
                    source = promptForPoint(PROMPTBOARDSELECTION);
                    if (source.size() == 1) {
                        // user wants to return to action menu
                        source.clear();
                        goto selectAction;
                    }
                } while (source.size() != 2);
                points =
                       getPossibleAttackOptionsForUnit(board, playerID, source);
                system("clear");
                printOpponentsHand(oppHandSize);
                healths = interface.getHqHealth();
                // TODO confirm which health is which
                printBoard(board, playerID, points, healths[playerID - 1],
                                                             healths[oppID -1]);
                printHand(handIDs);
                do {
                    dest = promptForPoint(PROMPTSECONDARYBOARDSELECTION);
                    if (dest.size() == 1) {
                        // user wants to return to action menu
                        dest.clear();
                        goto selectAction;
                    }
                } while (dest.size() != 2);
                interface.attack(source[0], source[1], dest[0], dest[1]);
            }
            break;
            default:
                //error
                cout << "Error!" << endl;
        }
        points.clear();

        board = safeGetBoardState(board, false);
        handSeeds = interface.getPlayerSeedHand(playerID);
        handIDs = buildHand(handSeeds);
        if (playerID == 1) {
            oppHand = interface.getPlayerSeedHand(2);
        }
        else {
            oppHand = interface.getPlayerSeedHand(1);
        }
        oppHandSize = oppHand.size();

        // Print initial turn state
        system("clear");
        printOpponentsHand(oppHandSize);

        points.clear();
        healths = interface.getHqHealth();
        // TODO confirm which health is which
        printBoard(board, playerID, points, healths[playerID - 1],
                                                             healths[oppID -1]);
        printHand(handIDs);
        if (!interface.isGameOver()) {
            interface.waitNextTurn();
        }


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

vector< vector< vector<uint8_t> > >
safeGetBoardState(vector< vector< vector<uint8_t> > >curBoard,
                                                      bool initialVerification){
    bool boardSound;
    vector< vector< vector<uint8_t> > >tmpBoard;
    do {
        boardSound = true;
        tmpBoard = interface.getBoardState();
        if (initialVerification) {
            for (uint i = 0; i < tmpBoard.size(); i++) {
                for (uint j = 0; j < tmpBoard[i].size(); j++) {
                    for (uint k = 0; k < tmpBoard[i][j].size(); k++) {
                        // too lazy to do this properly and efficiently
                        // maybe revisit
                        if (tmpBoard.size() * tmpBoard[i].size()
                                            * tmpBoard[i][j].size() != 270) {
                            boardSound = false;
                        }
                        if (tmpBoard[i][j][k] != 0) {
                            boardSound = false;
                        }
                    }
                }
            }
        }
        else {
            for (uint i = 0; i < tmpBoard.size(); i++) {
                int numDiffs = 0;
                for (uint j = 0; j < tmpBoard[i].size(); j++) {
                    for (uint k = 0; k < tmpBoard[i][j].size(); k++) {
                        if (tmpBoard.size() * tmpBoard[i].size()
                                            * tmpBoard[i][j].size() != 270) {
                            boardSound = false;
                        }
                        if (tmpBoard[i][j][k] != curBoard[i][j][k]) {
                            numDiffs++;
                        }
                    }
                }
                if (numDiffs > 2) {
                    boardSound = false;
                }
            }
        }
    } while (!boardSound);
    return tmpBoard;
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
    //vector< vector<uint8_t> > points =
    //                                getAllPathPlacementOptions(board, playerID);
    vector< vector<uint8_t> > points = getPossibleHQLocations(2);
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
    points = getAllTilesInControl(board, 2);
    printBoard(board, 2, points, healths[0], healths[1]);

    do {
        point = promptForPoint("Please enter a point\n");
    } while(point.size() != 2);
    printBoard(board, 2, points, healths[0], healths[1]);
    uint8_t adjx = 63; //error val
    uint8_t adjy = 63; //error val
    for(vector<uint8_t> tile:
                            getAllTilesInControl(board, playerID)) {
        for(vector<uint8_t> adjtile:
                                   getAdjacentTiles(point)) {
            if (tile[0] == adjtile[0] && tile[1] == adjtile[1]) {
                adjx = tile[0];
                adjy = tile[1];

            }
        }
    }
    points.clear();
    points.push_back({adjx, adjy});
    printBoard(board, 2, points, healths[0], healths[1]);
    int input = -1;
    do {
        input = promptForAction("Enter 1,2,3 or 4\n");
    } while(input == -1);
    input = -1;
    do {
        input = promptForCard("Enter 1,2,3 4, or 5\n", 5);
    } while(input == -1);

}


