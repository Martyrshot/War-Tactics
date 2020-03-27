// includes the function that get's all of the cards it needs
// cards from https://www.asciiart.eu/miscellaneous/playing-cards
//       by ejm98
#include <string>
#include <iostream>
#include <vector>

using namespace std;

#define CARDSPERSUIT 13
#define CARDHEIGHT 6

vector<string> cardback = {   
                         string(" _____ ")
                        ,string("|\\ ~ /|")
                        ,string("|}}:{{|")
                        ,string("|}}:{{|")
                        ,string("|}}:{{|")
                        ,string("|/_~_\\|")};
//Use this for HQ, but if have time, let's make it look cooler
vector<string> joker = {
                         string(" _____ ")
                        ,string("|Joker|")
                        ,string("|==%, |")
                        ,string("| \\/>\\|")
                        ,string("| _>^^|")
                        ,string("|/_\\/_|")};
      
vector< vector<string> > spades = {
                        {    string(" _____ ")
                            ,string("|A .  |")
                            ,string("| /.\\ |")
                            ,string("|(_._)|")
                            ,string("|  |  |")
                            ,string("|____V|")}
                        ,{   string(" _____ ")
                            ,string("|2    |")
                            ,string("|  ^  |")
                            ,string("|     |")
                            ,string("|  ^  |")
                            ,string("|____Z|")}

                        ,{   string(" _____ ")
                            ,string("|3    |")
                            ,string("| ^ ^ |")
                            ,string("|     |")
                            ,string("|  ^  |")
                            ,string("|____E|")}

                        ,{   string(" _____ ")
                            ,string("|4    |")
                            ,string("| ^ ^ |")
                            ,string("|     |")
                            ,string("| ^ ^ |")
                            ,string("|____h|")}

                        ,{   string(" _____ ")
                            ,string("|5    |")
                            ,string("| ^ ^ |")
                            ,string("|  ^  |")
                            ,string("| ^ ^ |")
                            ,string("|____S|")}

                        ,{   string(" _____ ")
                            ,string("|6    |")
                            ,string("| ^ ^ |")
                            ,string("| ^ ^ |")
                            ,string("| ^ ^ |")
                            ,string("|____9|")}

                        ,{   string(" _____ ")
                            ,string("|7    |")
                            ,string("| ^ ^ |")
                            ,string("|^ ^ ^|")
                            ,string("| ^ ^ |")
                            ,string("|____L|")}

                        ,{   string(" _____ ")
                            ,string("|8    |")
                            ,string("|^ ^ ^|")
                            ,string("| ^ ^ |")
                            ,string("|^ ^ ^|")
                            ,string("|____8|")}

                        ,{   string(" _____ ")
                            ,string("|9    |")
                            ,string("|^ ^ ^|")
                            ,string("|^ ^ ^|")
                            ,string("|^ ^ ^|")
                            ,string("|____6|")}

                        ,{   string(" _____ ")
                            ,string("|10 ^ |")
                            ,string("|^ ^ ^|")
                            ,string("|^ ^ ^|")
                            ,string("|^ ^ ^|")
                            ,string("|___0I|")}

                        ,{   string(" _____ ")
                            ,string("|J  ww|")
                            ,string("| ^ {)|")
                            ,string("|(.)% |")
                            ,string("| | % |")
                            ,string("|__%%[|")}

                        ,{   string(" _____ ")
                            ,string("|Q  ww|")
                            ,string("| ^ {(|")
                            ,string("|(.)%%|")
                            ,string("| |%%%|")
                            ,string("|_%%%O|")}

                       ,{    string(" _____ ")
                            ,string("|K  WW|")
                            ,string("| ^ {)|")
                            ,string("|(.)%%|")
                            ,string("| |%%%|")
                            ,string("|_%%%>|")}};
   
      
vector< vector<string> > clubs = {
                        {    string(" _____ ")
                            ,string("|A _  |")
                            ,string("| ( ) |")
                            ,string("|(_'_)|")
                            ,string("|  |  |")
                            ,string("|____V|")}
    
                        ,{   string(" _____ ")
                            ,string("|2    |")
                            ,string("|  &  |")
                            ,string("|     |")
                            ,string("|  &  |")
                            ,string("|____Z|")}

                        ,{   string(" _____ ")
                            ,string("|3    |")
                            ,string("| & & |")
                            ,string("|     |")
                            ,string("|  &  |")
                            ,string("|____E|")}

                        ,{   string(" _____ ")
                            ,string("|4    |")
                            ,string("| & & |")
                            ,string("|     |")
                            ,string("| & & |")
                            ,string("|____h|")}
                        
                        ,{   string(" _____ ")
                            ,string("|5    |")
                            ,string("| & & |")
                            ,string("|  &  |")
                            ,string("| & & |")
                            ,string("|____S|")}
                        
                        ,{   string(" _____ ")
                            ,string("|6    |")
                            ,string("| & & |")
                            ,string("| & & |")
                            ,string("| & & |")
                            ,string("|____9|")}

                        ,{   string(" _____ ")
                            ,string("|7    |")
                            ,string("| & & |")
                            ,string("|& & &|")
                            ,string("| & & |")
                            ,string("|____L|")}

                        ,{   string(" _____ ")
                            ,string("|8    |")
                            ,string("|& & &|")
                            ,string("| & & |")
                            ,string("|& & &|")
                            ,string("|____8|")}

                        ,{   string(" _____ ")
                            ,string("|9    |")
                            ,string("|& & &|")
                            ,string("|& & &|")
                            ,string("|& & &|")
                            ,string("|____6|")}

                        ,{   string(" _____ ")
                            ,string("|10 & |")
                            ,string("|& & &|")
                            ,string("|& & &|")
                            ,string("|& & &|")
                            ,string("|___0I|")}

                        ,{   string(" _____ ")
                            ,string("|J  ww|")
                            ,string("| o {)|")
                            ,string("|o o% |")
                            ,string("| | % |")
                            ,string("|__%%[|")}

                        ,{   string(" _____ ")
                            ,string("|Q  ww|")
                            ,string("| o {(|")
                            ,string("|o o%%|")
                            ,string("| |%%%|")
                            ,string("|_%%%O|")}

                        ,{   string(" _____ ")
                            ,string("|K  WW|")
                            ,string("| o {)|")
                            ,string("|o o%%|")
                            ,string("| |%%%|")
                            ,string("|_%%%>|")}};

vector< vector<string> > hearts = {
                        {    string(" _____ ")
                            ,string("|A_ _ |")
                            ,string("|( v )|")
                            ,string("| \\ / |")
                            ,string("|  .  |")
                            ,string("|____V|")}
                            
                        ,{   string(" _____ ")
                            ,string("|2    |")
                            ,string("|  v  |")
                            ,string("|     |")
                            ,string("|  v  |")
                            ,string("|____Z|")}
                        
                        ,{   string(" _____ ")
                            ,string("|3    |")
                            ,string("| v v |")
                            ,string("|     |")
                            ,string("|  v  |")
                            ,string("|____E|")}
                        
                        ,{   string(" _____ ")
                            ,string("|4    |")
                            ,string("| v v |")
                            ,string("|     |")
                            ,string("| v v |")
                            ,string("|____h|")}
                        
                        ,{   string(" _____ ")
                            ,string("|5    |")
                            ,string("| v v |")
                            ,string("|  v  |")
                            ,string("| v v |")
                            ,string("|____S|")}
                        
                        ,{   string(" _____ ")
                            ,string("|6    |")
                            ,string("| v v |")
                            ,string("| v v |")
                            ,string("| v v |")
                            ,string("|____9|")}
                        
                        ,{   string(" _____ ")
                            ,string("|7    |")
                            ,string("| v v |")
                            ,string("|v v v|")
                            ,string("| v v |")
                            ,string("|____L|")}
                        
                        ,{   string(" _____ ")
                            ,string("|8    |")
                            ,string("|v v v|")
                            ,string("| v v |")
                            ,string("|v v v|")
                            ,string("|____8|")}
                            
                        ,{   string(" _____ ")
                            ,string("|9    |")
                            ,string("|v v v|")
                            ,string("|v v v|")
                            ,string("|v v v|")
                            ,string("|____6|")}
                        
                        ,{   string(" _____ ")
                            ,string("|10 v |")
                            ,string("|v v v|")
                            ,string("|v v v|")
                            ,string("|v v v|")
                            ,string("|___0I|")}
                        
                        ,{   string(" _____ ")
                            ,string("|J  ww|")
                            ,string("|   {)|")
                            ,string("|(v)% |")
                            ,string("| v % |")
                            ,string("|__%%[|")}
                            
                        ,{   string(" _____ ")
                            ,string("|Q  ww|")
                            ,string("|   {(|")
                            ,string("|(v)%%|")
                            ,string("| v%%%|")
                            ,string("|_%%%O|")}
                            
                        ,{   string(" _____ ")
                            ,string("|K  WW|")
                            ,string("|   {)|")
                            ,string("|(v)%%|")
                            ,string("| v%%%|")
                            ,string("|_%%%>|")}   };

vector< vector<string> > diamonds = {
                        {    string(" _____ ")
                            ,string("|A ^  |")
                            ,string("| / \\ |")
                            ,string("| \\ / |")
                            ,string("|  .  |")
                            ,string("|____V|")}
                            
                        ,{   string(" _____ ")
                            ,string("|2    |")
                            ,string("|  o  |")
                            ,string("|     |")
                            ,string("|  o  |")
                            ,string("|____Z|")}
                            
                        ,{   string(" _____ ")
                            ,string("|3    |")
                            ,string("| o o |")
                            ,string("|     |")
                            ,string("|  o  |")
                            ,string("|____E|")}
                        
                        ,{   string(" _____ ")
                            ,string("|4    |")
                            ,string("| o o |")
                            ,string("|     |")
                            ,string("| o o |")
                            ,string("|____h|")}
                            
                        ,{   string(" _____ ")
                            ,string("|5    |")
                            ,string("| o o |")
                            ,string("|  o  |")
                            ,string("| o o |")
                            ,string("|____S|")}
                            
                        ,{   string(" _____ ")
                            ,string("|6    |")
                            ,string("| o o |")
                            ,string("| o o |")
                            ,string("| o o |")
                            ,string("|____9|")}
                            
                        ,{   string(" _____ ")
                            ,string("|7    |")
                            ,string("| o o |")
                            ,string("|o o o|")
                            ,string("| o o |")
                            ,string("|____L|")}
                            
                        ,{   string(" _____ ")
                            ,string("|8    |")
                            ,string("|o o o|")
                            ,string("| o o |")
                            ,string("|o o o|")
                            ,string("|____8|")}

                        ,{   string(" _____ ")
                            ,string("|9    |")
                            ,string("|o o o|")
                            ,string("|o o o|")
                            ,string("|o o o|")
                            ,string("|____6|")}
                            
                        ,{   string(" _____ ")
                            ,string("|10 o |")
                            ,string("|o o o|")
                            ,string("|o o o|")
                            ,string("|o o o|")
                            ,string("|___0I|")}
                            
                        ,{   string(" _____ ")
                            ,string("|J  ww|")
                            ,string("| /\\{)|")
                            ,string("| \\/% |")
                            ,string("|   % |")
                            ,string("|__%%[|")}
                            
                        ,{   string(" _____ ")
                            ,string("|Q  ww|")
                            ,string("| /\\{(|")
                            ,string("| \\/%%|")
                            ,string("|  %%%|")
                            ,string("|_%%%O|")}
                            
                        ,{   string(" _____ ")
                            ,string("|K  WW|")
                            ,string("| /\\{)|")
                            ,string("| \\/%%|")
                            ,string("|  %%%|")
                            ,string("|_%%%>|")}};


// It is the responsibility of the caller to pass shifted number that starts at
// 0.
void printHand(vector<uint8_t> ids, uint8_t idx) {

    vector< vector<string> > cards;
    for(int n : ids) {
        if (n < 0) {
            return;
        }
        if (n < 13) {
            cards.push_back(spades[n]);
        }
        else if (n < 26) {
            cards.push_back(clubs[n%13]);
        }
        else if (n < 39) {
            cards.push_back(diamonds[n%13]);
        }
        else {
            cards.push_back(hearts[n%13]);
        }
    }

    
    for (int i = 0; i < CARDHEIGHT; i++) {
        int curCard = 0;
        for (vector<string> j: cards) {
            if (idx == curCard){
                cout << "\033[1;33m" << j[i] << "\033[0m";
            }
            else {
                cout << j[i];
            }
            curCard++;
        }
        cout << endl;
    }
    for (size_t i = 0; i < cards.size(); i++) {
        cout << "   " << i + 1<< "   ";
    }
    cout << endl;
}

void printHand(vector<uint8_t> ids) {
    printHand(ids, -1);
}

void printOpponentsHand(uint8_t n) {
    for (int i = 0; i < CARDHEIGHT; i++) {
        for (int j = 0; j < n; j++) {
            cout << cardback[i];
        }
        cout << endl;
    }
}