//
// Game Recommender | by: Erik Companhone and Liam McKenna.
// Project 3 for COP3530 - Aug 2023.
//

#include <iostream>
#include <istream>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <cstring>
#include "dist/jsoncpp.cpp"
#include <curl/curl.h>
#include "bptree.h"

using namespace std;

Json::Value GetSteamProfile();
map<string, int> GetGenres();
void GetTopTenOrderedMap(map<int, string> gameMap);
size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s);
map<int, string> CreateRBTreeValues(map<string, int> genres);
bptree CreateBPTreeValues(map<string, int> genres);
void displayTotalPlayTime(map<string,int> genre);

//Steam ID: 76561198355838737

int main()
{
    //Recommends a game based on metacritic score and user's steam account preferences.

    //Get valid steam ID.
    bool validID = false;
    Json::Value userData;
    while (!validID) {
        userData = GetSteamProfile();
        if (userData["response"]["game_count"].asInt() > 1) { //checks if there is at least one game in the user's library
            validID = true;
            cout << "\nWe found your profile!\n" << endl;
        } else {
            cout << "We could not access game data from this profile.\nPlease ensure your Steam ID is correct and your profile and game details is set to public." << endl;
        }
    }

    //Get map of genres that will house their respective playtimes
    map<string, int> genres = GetGenres();
    CURL *curl;
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
    cout << "Generating game preferences..." << endl;
    //iterates through each game and finds its respective match in the database
    for (int i = 0; i <  userData["response"]["game_count"].asInt(); i++)
    {
        string data;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
        string name = userData["response"]["games"][i]["name"].asString();
        //replaces abstract characters to ensure url is valid
        std::replace( name.begin(), name.end(), ' ', '+');
        std::replace( name.begin(), name.end(), '\'', '+');
        std::replace( name.begin(), name.end(), '(', '+');
        std::replace( name.begin(), name.end(), ')', '+');

        //performs api call and deserialization of json data
        string gameSearch = "https://api.rawg.io/api/games?key=efca16b089bf46f993dae2b1df78851c&stores=1&exclude_additions=true&page=1&page_size=1&search=" + name;
        Json::Value game;
        Json::Reader reader1;
        curl_easy_setopt(curl, CURLOPT_URL, gameSearch.c_str());
        curl_easy_perform(curl);
        reader1.parse(data, game);
        //iterates through each game's genre to add to the genre map
        for (int j = 0; j < game["results"][0]["genres"].size(); j++) {
            genres[game["results"][0]["genres"][j]["name"].asString()] = genres[game["results"][0]["genres"][j]["name"].asString()] + (userData["response"]["games"][i]["playtime_forever"].asInt() / game["results"][0]["genres"].size());
        }
    }
    cout << "Profile loaded!" << endl;
    cout << endl;
    displayTotalPlayTime(genres);

    //User chooses between two maps.
    std::chrono::duration<double> operationTime;
    string selection = "0";
    bptree gameBPTree(1001);
    std::map<int, string> gameMap;
    while (stoi(selection) != 1 && stoi(selection) != 2)
    {
        cout << "Which tree would you like to use?\n1 - Red & Black tree (weighted genre preference)\n2 - B+ tree (unweighted genre preference)" << endl;
        cin >> selection;
        if (stoi(selection) == 1) //Call red/black tree version.
        {
            std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
            gameMap = CreateRBTreeValues(genres);
            std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
            operationTime = end - start;
            cout << "\nRed Black Tree insert time: " << operationTime.count()/1000.0 << " seconds.\n" << endl;
            GetTopTenOrderedMap(gameMap);
        }
        else if (stoi(selection) == 2) //Call B+ tree version.
        {
            std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
            gameBPTree = CreateBPTreeValues(genres);
            std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
            cout << "\nB+ Tree insert time: " << operationTime.count()/1000.0 << " seconds.\n" << endl;
            gameBPTree.getTop10(gameBPTree.getRoot());
        }
        else
        {
            cout << "Invalid selection. Please try again." << endl;
        }
    }

    curl_global_cleanup();
    return 0;
}

Json::Value GetSteamProfile()
{
    //This method returns a json value of the user's steam profile.
    string steamID;
    Json::Value profileValue;
    cout << "Enter Steam ID: ";
    cin >> steamID;

    //ensures digits
    for (int i = 0; i < steamID.length(); i++) {
        if (isdigit(steamID[i])) continue;
        else return profileValue;
    }

    //prepares api call and deserializes returned json data for steam profile
    CURL *curl;
    Json::Reader profileReader;
    CURLcode result;
    curl = curl_easy_init();
    string profileReturn;
    string profileURL = "http://api.steampowered.com/IPlayerService/GetOwnedGames/v0001/?key=5E1E03006C1C57FA2B2520761846DAF3&format=json&include_played_free_games=true&include_appinfo=true&steamid=" + steamID;

    /* set URL to operate on */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
    curl_easy_setopt(curl, CURLOPT_URL, profileURL.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &profileReturn);
    result = curl_easy_perform(curl);
    profileReader.parse(profileReturn, profileValue);

    return profileValue;
}

size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s)
{
    //ensures data returned from api call is of type string
    size_t newLength = size*nmemb;
    try
    {
        s->append((char*)contents, newLength);
    }
    catch(std::bad_alloc &e)
    {
        //handle memory problem
        return 0;
    }
    return newLength;
}

map<string, int> GetGenres()
{
    //This method gets complete list of genres according to RAWG database and maps them
    //prepares and completes api call for list of genres
    map<string, int> genres;
    string genreList;
    Json::Value genreListValue;
    Json::Reader genreReader;
    CURL *curl;
    curl = curl_easy_init();

    string getGenresURL = "https://api.rawg.io/api/genres?key=efca16b089bf46f993dae2b1df78851c";
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
    curl_easy_setopt(curl, CURLOPT_URL, getGenresURL.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &genreList);
    genreReader.parse(genreList.c_str(), genreListValue);

    //maps genres
    for (int i = 0; i < genreListValue["count"].asInt(); i++) {
        genres[genreListValue["results"][i]["name"].asString()] = 0;
    }

    return genres;
}

bptree CreateBPTreeValues(map<string, int> genres)
{
    //creates values for b plus tree
    //parses pre-existing json database containing 120k games into json value
    cout << "Parsing json. This may take a while..." << endl;
    ifstream ifs("database.json");
    bptree gameBPTree(1001);
    Json::Value gamesListValue;
    ifs >> gamesListValue;

    cout << "Calculating your games list..." << endl;
    //iterates through games database and assigns each one a rating based off genre compatibility and metacritic score
    for (int i = 0; i < gamesListValue.size()/3; i++)
    {
        int rating = 0;
        string name = gamesListValue[i]["name"].asString();
        for (int j = 0; j < gamesListValue[i]["genres"].size(); j++)
        {
            rating += genres[gamesListValue[i]["genres"][j]["name"].asString()];
        }
        if (gamesListValue[i]["metacritic"].asInt() == 0) {
            //if no metacritic exists, we give it the average metacritic rating.
            rating *= 62;
        }
        else
        {
            rating *= gamesListValue[i]["metacritic"].asInt();
        }
        gameBPTree.insert(gameData(rating, name));
    }
    return gameBPTree;
}

map<int, string> CreateRBTreeValues(map<string, int> genres)
{
    //creates values for red black tree
    //parses pre-existing json database containing 120k games into json value
    map<int, string> orderedMap;
    cout << "Parsing json. This may take a while..." << endl;
    ifstream ifs("database.json");
    Json::Value gamesListValue;
    ifs >> gamesListValue;
    cout << "Calculating your games list..." << endl;
    //iterates through games database and assigns each one a rating based off genre compatibility and metacritic score
    int playedGenres = 0;
    int rating;
    string name;
    for (int i = 0; i < gamesListValue.size()/3; i++) {
        rating = 0;
        name = gamesListValue[i]["name"].asString();
        for (int j = 0; j < gamesListValue[i]["genres"].size(); j++)
        {
            if(genres[gamesListValue[i]["genres"][j]["name"].asString()] > 0)
            {
                rating += genres[gamesListValue[i]["genres"][j]["name"].asString()];
                playedGenres++;
            }
        }
        rating = (int)(rating/playedGenres);
        if (gamesListValue[i]["metacritic"].asInt() == 0)
        {
            //if no metacritic exists, we give it the average metacritic rating.
            rating *= 62;
        }
        else
        {
            rating *= gamesListValue[i]["metacritic"].asInt();
        }
        orderedMap[rating] = name;
    }
    return orderedMap;
}

void GetTopTenOrderedMap(map<int, string> gameMap)
{
    //prints top 10 highest rated games from red black tree.
    map<int, string>::reverse_iterator it;
    int j = 0;
    cout << "Here are 10 games we think you should try:" << endl;
    for (it = gameMap.rbegin(); it != gameMap.rend(); it++) {
        j++;
        if (j == 11) break;
        cout << j << ". " << it->second << endl;
    }
}

void displayTotalPlayTime(map<string,int> genres)
{
    //This method displays the total playtime for each genre the player has played.
    int totalPlaytime = 0;
    //determines total playtime across all genres
    for (auto i : genres) {
        totalPlaytime += i.second;
    }
    //displays relative playtime per genre
    cout << "Relative playtime per genre:" << endl;
    for (auto i : genres) {
        cout << i.first << ": ";
        fprintf(stdout, "%.2f", ((float)i.second / (float)totalPlaytime) * 100);
        cout << "%" << endl;
    }
    cout << endl;
}
