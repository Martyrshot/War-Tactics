#include <string>
#include <iostream>
#include <vector>

using namespace std;

void promptForEnter(string stringToPrint) {
    cout << stringToPrint << flush;
    while (cin.get() != '\n');
}

vector<int> promptForPoint(string stringToPrint) {
    string input;
    vector<int> result;
    cout << stringToPrint << flush;
    cin >> input;

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

int promptForAction(string stringToPrint) {
    int input = -1;
    cout << stringToPrint << flush;
    cin >> input;
    input = input - 1;
    if (input > 3 || input < 0) {
        cout << "Bad input!\n" << flush;
        return -1;
    }
    return input;
}

int promptForCard(string stringToPrint, int numCards) {
    int input = -1;
    cout << stringToPrint << flush;
    cin >> input;
    input = input - 1;
    if (input > numCards - 1 || input < 0) {
        cout << "Bad input!\n" << flush;
        return -1;
    }
    return input;
}