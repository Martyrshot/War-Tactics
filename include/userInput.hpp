#include <string>
#include <iostream>
#include <vector>

using namespace std;

void promptForEnter(string stringToPrint) {
    cout << stringToPrint << flush;
    cin.clear();
    while (cin.get() != '\n');
    
}

vector<uint8_t> promptForPoint(string stringToPrint) {
    string input;
    vector<uint8_t> result;
    cout << stringToPrint << flush;
    cin >> input;
    cin.clear();

    if (input.size() != 2) {
        cout << "Bad input!\n" << flush;
        return result;
    }
    // first try uppercase
    int x = input[0] - 65;
    if (x > 9 || x < 0) {
        // wasn't uppercase, try lowercase
        x = input[0] - 97;
        if (x > 9 || x < 0) {
            cout << "Bad input!\n" << flush;
            return result;
        }
    }

    // convert to digit and shift back to 0
    int y = (input[1] -48 )-1;
    if (y > 9 || y < 0) {
        cout << "Bad input!\n" << flush;
        return result;
    }
    result.push_back(x);
    result.push_back(y);
    return result;
}

int8_t promptForAction(string stringToPrint) {
    int8_t input = -1;
    cout << stringToPrint << flush;
    cin.clear();
    cin >> input;
    input = input - 49;
    cout << (int)input << endl;
    if (input > 3 || input < 0) {
        cout << "Bad input!\n" << flush;
        return -1;
    }
    return input;
}

int8_t promptForCard(string stringToPrint, int numCards) {
    int input = -1;
    cout << stringToPrint << flush;
    cin >> input;
    cin.clear();

    input = input - 1;
    if (input > numCards - 1 || input < 0) {
        cout << "Bad input!\n" << flush;
        return -1;
    }
    return input;
}


int8_t mainMenu(string stringToPrint) {
    int input = -1;
    cout << stringToPrint << endl << endl << endl << "Selection an option:";
    cout << endl << "1) make a game" << endl << "2) join a game" << endl;
    cin >> input;
    if (input > 2 || input < 1) {
        cout << "Bad input!" << endl;
        return -1;
    }
    return input - 1;
}

string promptAddress(void) {
    string input;
    cout << "Please enter the address of the game you wish to join:" << endl;
    cin >> input;
    return input;
}
