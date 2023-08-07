//
// Game Recommender | by: Erik Companhone and Liam McKenna.
// Project 3 for COP3530 - Aug 2023.
//
#include "string"

#ifndef BPTREE_GAMEDATA_H
#define BPTREE_GAMEDATA_H


class gameData
{
    int rating;
    std::string genre;
    std::string name;

public:
    gameData();
    gameData(int rating, std::string name);
    gameData(int rating);
    int getRating();
    std::string getName();
    std::string getGenre();
};


#endif //BPTREE_GAMEDATA_H