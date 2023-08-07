//
// Game Recommender | by: Erik Companhone and Liam McKenna.
// Project 3 for COP3530 - Aug 2023.
//

#include "gameData.h"

gameData::gameData(int rating, std::string name)
{
    //This method is the constructor of the gameData object.
    this->rating = rating;
    this->name = name;

}

int gameData::getRating()
{
    //This method returns the rating of the game.
    return rating;
}

std::string gameData::getName()
{
    //This method returns the name of the game.
    return name;
}

std::string gameData::getGenre()
{
    //This method retrieves the genre of the game.
    return genre;
}

gameData::gameData(int rating)
{
    //This method is a constructor that only takes in a rating value. Used for testing.
    this->rating = rating;
    this->genre = "Action-Adventure";
    this->name = "Shadow of the Colossus";
}

gameData::gameData()
{
    //Default constructor for array initialization.
    //this->rating = 100;
    //this->genre = "Action-Adventure";
    //this->name = "Shadow of the Colossus";
}