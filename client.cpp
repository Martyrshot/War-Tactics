#include <string>
#include <vector>
#include "include/cards.hpp"
using namespace std;



int main(int argc, char **argv) {
    vector<int> v;
    for (int i = 0; i < 52; i++) {
        if (i %13 == 2) {
            v.push_back(i);
        } 
    }

    vector< vector<string> > cards = idsToASCII(v);
    printCards(cards);
    return 0;
}