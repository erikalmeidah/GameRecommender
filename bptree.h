//
// Game Recommender | by: Erik Companhone and Liam McKenna.
// Project 3 for COP3530 - Aug 2023.
//

#include "node.h"
#include "gameData.h"

#ifndef BPTREE_BPTREE_H
#define BPTREE_BPTREE_H


class bptree
{
    node* root;
    int maxDegree;

public:
    bptree(int deg);
    node* getRoot();
    int getDegree();
    void insert(gameData myGame);
    void getTop10(node* root);
};


#endif //BPTREE_BPTREE_H