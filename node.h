//
// Game Recommender | by: Erik Companhone and Liam McKenna.
// Project 3 for COP3530 - Aug 2023.
//

#include "gameData.h"
#include <vector>

#ifndef BPTREE_NODE_H
#define BPTREE_NODE_H


class node
{
public:
    bool leaf;
    gameData* dataArray;
    int size;
    int degree;
    node** child = nullptr;

private:
    node(int maxNodes);
    int getDegree();
    int simpleInsert(gameData myGame);
    node* internalInsert(node* root, node* parent, node* newLeft, node* newRight, const gameData& myGame, gameData midVal, std::vector<node*> &parents);

    friend class bptree; //No need for setters and getters.
};


#endif //BPTREE_NODE_H