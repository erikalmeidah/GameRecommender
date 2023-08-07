//
// Game Recommender | by: Erik Companhone and Liam McKenna.
// Project 3 for COP3530 - Aug 2023.
//

#include "node.h"
#include "iostream"
#include <utility>

node::node(int maxNodes)
{
    //This is the constructor for a node.
    degree = maxNodes + 1;
    dataArray = new gameData[maxNodes];
    child = new node*[degree];
    size = 0;
}

int node::getDegree()
{
    //This returns the degree of the node.
    return degree;
}

int node::simpleInsert(gameData myGame)
{
    //This method inserts a game into the node's dataArray based on ascending rating order.
    int pos = 0;
    int maxNodes = degree - 1;
    int rating = myGame.getRating();

    //Get position to insert in.
    for(int i = 0; i < size; i++)
    {
        if(dataArray[i].getRating() < rating)
        {
            pos++;
        }
        else
        {
            break;
        }
    }

    //Shift array to make space for new rating.
    for(int i = maxNodes -1; i > pos; i--)
    {
        dataArray[i] = dataArray[i - 1];
    }

    //Insert.
    this->dataArray[pos] = myGame;
    size++;
    return pos;
}

node* node::internalInsert(node* root, node* parent, node* newLeft, node* newRight, const gameData& myGame, gameData midVal, std::vector<node*> &parents)
{
    //This method is used to recursively insert when internal nodes are full.
    node* parentNode = parent;
    node* currentNode = this;
    int maxNodes = parentNode->degree - 1;
    gameData midValue = std::move(midVal);

    //root is not full
    if(parentNode->size < maxNodes && parentNode == root)
    {
        int pos = parentNode->simpleInsert(midValue);
        //Update pointers.
        parentNode->child[pos] = newLeft;
        //Check for right leaf.
        if(parentNode->child[pos + 1] == nullptr)
        {
            parentNode->child[pos + 1] = newRight;
        }
            //There is a leaf at the right of pos.
        else
        {
            //shift child pointers to insert correctly
            for(int i = parentNode->size; i > pos; i--)
            {
                parentNode->child[i + 1] = parentNode->child[i];
            }
            parentNode->child[pos + 1] = newRight;
        }
        return parentNode;
    }

    //Temp insert.
    gameData parentTempData[maxNodes + 1];
    int parentMidPos = maxNodes / 2;
    gameData parentMidValue;
    for(int i = 0 ; i < maxNodes; i++)
    {
        parentTempData[i] = parentNode->dataArray[i];
        if(i == parentMidPos)
        {
            parentMidValue = parentNode->dataArray[i];
        }
    }

    //Get position to insert in.
    int tempParentPos = 0;
    for(int i = 0; i < maxNodes; i++)
    {
        if(parentTempData[i].getRating() < midValue.getRating())
        {
            tempParentPos++;
        }
        else
        {
            break;
        }
    }

    //Shift array to make space for new myGame.
    for(int i = maxNodes; i > tempParentPos; i--)
    {
        parentTempData[i] = parentTempData[i - 1];
    }

    //Insert in temp array.
    parentTempData[tempParentPos] = midValue;

    //Split parent.
    node* newParent = new node(maxNodes);
    int parentPos = newParent->simpleInsert(parentMidValue);
    node* newLeftInternalNode = new node(maxNodes);
    node* newRightInternalNode = new node(maxNodes);
    for(int i = 0; i < degree/2; i++)
    {
        newLeftInternalNode->dataArray[i] = parentTempData[i];
        newLeftInternalNode->size++;
    }
    int j = 0;
    for(int i = degree/2 + 1; i < degree; i++)
    {
        newRightInternalNode->dataArray[j] = parentTempData[i];
        newRightInternalNode->size++;
        j++;
    }
    newLeftInternalNode->leaf = false;
    newRightInternalNode->leaf = false;

    //reorganize left child pointers.
    int k = 0;
    bool foundCurr = false;
    for(int i = 0; i < newLeftInternalNode->size + 1; i++)
    {
        //Node that was previously split.
        if(parentNode->child[i] == currentNode)
        {
            newLeftInternalNode->child[i] = newLeft;
            newLeftInternalNode->child[i + 1] = newRight;
            i++;
            k++;
            foundCurr = true;
        }
        else
        {
            if(foundCurr)
            {
                newLeftInternalNode->child[i] = parentNode->child[i - 1];
            }
            else
            {
                newLeftInternalNode->child[i] = parentNode->child[i];
            }
            k++;
        }
    }

    //reorganize right child pointers.
    foundCurr = false;
    for(int i = 0; i < newRightInternalNode->size + 1; i++)
    {
        //Node that was previously split.
        if(parentNode->child[k] == currentNode)
        {
            newRightInternalNode->child[i] = newLeft;
            newRightInternalNode->child[i + 1] = newRight;
            i++;
            foundCurr = true;
        }
        else
        {
            if(foundCurr)
            {
                newRightInternalNode->child[i] = parentNode->child[i + ((maxNodes + 1)/2) - 1];
            }
            else
            {
                newRightInternalNode->child[i] = parentNode->child[i + ((maxNodes + 1)/2)];
            }
        }
    }

    //Base case: parent is root.
    if(parentNode == root)
    {
        //Root is full.
        //Reorganize parent.
        newParent->child[0] = newLeftInternalNode;
        newParent->child[1] = newRightInternalNode;
        newParent->size = 1;
        newParent->leaf = false;
        root = newParent;
        return root;
    }
        //Case 2: parent is internal node and is full.
    else
    {
        //Get next parent.
        node* recursiveParent = parents.back();
        if(!parents.empty())
            parents.pop_back();
        //Recursive call here.
        root = newParent->internalInsert(root, recursiveParent, newLeftInternalNode, newRightInternalNode, midValue, parentMidValue, parents);
        return root;
    }
}