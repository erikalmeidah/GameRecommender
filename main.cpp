//
// Game Recommender | by: Erik Companhone and Liam McKenna.
// Project 3 for COP3530 - Aug 2023.
//

#include <iostream>
#include <istream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include "dist/jsoncpp.cpp"
#include <curl/curl.h>
#include "bptree.h"
#include <chrono>


using namespace std;
Json::Value GetSteamProfile();
map<string, int> GetGenres();
void GetTopTenOrderedMap(map<int, string> gameMap);
size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s);
map<int, string> CreateRBTreeValues(map<string, int> genres, chrono::milliseconds& operationTime);
bptree CreateBPTreeValues(map<string, int> genres, chrono::milliseconds& operationTime);

int main() {

    bool validID = false;
    Json::Value userData;
    while (!validID) {
        userData = GetSteamProfile();
        if (userData["response"]["game_count"].asInt() > 1) { //checks if there is at least one game in the user's library
            validID = true;
            cout << "We found your profile!" << endl;
        } else {
            cout << "We could not access game data from this profile.\nPlease ensure your Steam ID is correct and your profile and game details is set to public." << endl;
        }
    }

    //gets map of genres that will house their respective playtimes
    map<string, int> genres = GetGenres();

    //setting up api call
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);

    cout << "Generating game preferences..." << endl;
    //iterates through each game and finds its respective match in the database
    for (int i = 0; i <  userData["response"]["game_count"].asInt(); i++) {
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
        reader1.parse(data.c_str(), game);
        //iterates through each game's genre to add to the genre map
        for (int j = 0; j < game["results"][0]["genres"].size(); j++) {
            genres[game["results"][0]["genres"][j]["name"].asString()] = genres[game["results"][0]["genres"][j]["name"].asString()] + (userData["response"]["games"][i]["playtime_forever"].asInt() / game["results"][0]["genres"].size());
        }
    }

    cout << "Profile loaded!" << endl;
    cout << endl;
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

    //user chooses between two maps
    chrono::milliseconds operationTime;
    string selection = "0";
    bptree gameBPTree(1001);
    std::map<int, string> gameMap;
    while (stoi(selection) != 1 && stoi(selection) != 2) {
        cout << "Which tree would you like to use? Press 1 for Red & Black, 2 for B+" << endl;
        cin >> selection;
        if (stoi(selection) == 1) {
            gameMap = CreateRBTreeValues(genres, operationTime);
            cout << "Red Black Tree insert time: " << operationTime.count()/1000.0 << " seconds." << endl;
            GetTopTenOrderedMap(gameMap);
        } else if (stoi(selection) == 2) {
            gameBPTree = CreateBPTreeValues(genres, operationTime);
            cout << "B+ Tree insert time: " << operationTime.count()/1000.0 << " seconds." << endl;
            gameBPTree.getTop10(gameBPTree.getRoot());
        } else{
            cout << "Invalid selection. Please try again." << endl;
        }
    }

    curl_global_cleanup();
    return 0;
}

Json::Value GetSteamProfile() {
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
    profileReader.parse(profileReturn.c_str(), profileValue);

    return profileValue;
}

//ensures data returned from api call is of type string
size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s)
{
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

//gets complete list of genres according to RAWG database and maps them
map<string, int> GetGenres() {
    //prepares and completes api call for list of genres
    map<string, int> genres;
    string genreList;
    Json::Value genreListValue;
    Json::Reader genreReader;
    CURL *curl;
    curl = curl_easy_init();
    CURLcode result;
    string getGenresURL = "https://api.rawg.io/api/genres?key=efca16b089bf46f993dae2b1df78851c";
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
    curl_easy_setopt(curl, CURLOPT_URL, getGenresURL.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &genreList);
    result = curl_easy_perform(curl);
    genreReader.parse(genreList.c_str(), genreListValue);

    //maps genres
    for (int i = 0; i < genreListValue["count"].asInt(); i++) {
        genres[genreListValue["results"][i]["name"].asString()] = 0;
    }

    return genres;
}
//creates values for b plus tree
bptree CreateBPTreeValues(map<string, int> genres, chrono::milliseconds& operationTime) {
    //parses pre-existing json database containing 120k games into json value
    cout << "Parsing json. This may take a while..." << endl;
    ifstream ifs("database.json");
    bptree gameBPTree(1001);
    Json::Value gamesListValue;
    ifs >> gamesListValue;

    cout << "Calculating your games list..." << endl;
    //starts clock
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = chrono::high_resolution_clock::now();
    //iterates through games database and assigns each one a rating based off genre compatibility and metacritic score
    for (int i = 0; i < gamesListValue.size(); i++) {
        int rating = 0;
        string name = gamesListValue[i]["name"].asString();
        for (int j = 0; j < gamesListValue[i]["genres"].size(); j++) {
            rating += genres[gamesListValue[i]["genres"][j]["name"].asString()];
        }
        if (gamesListValue[i]["metacritic"].asInt() == 0) {
            //if no metacritic exists, we give it the average metacritic rating.
            rating *= 62;
        } else {
            rating *= gamesListValue[i]["metacritic"].asInt();
        }
        gameBPTree.insert(gameData(rating, name));
    }
    //stops clock
    end = chrono::high_resolution_clock::now();
    operationTime = chrono::duration_cast<chrono::milliseconds>(end - start);

    return gameBPTree;
}

//creates values for red black tree

map<int, string> CreateRBTreeValues(map<string, int> genres, chrono::milliseconds& operationTime) {
    //parses pre-existing json database containing 120k games into json value
    map<int, string> orderedMap;
    cout << "Parsing json. This may take a while..." << endl;
    ifstream ifs("database.json");
    Json::Value gamesListValue;
    ifs >> gamesListValue;
    cout << "Calculating your games list..." << endl;
    //starts clock
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = chrono::high_resolution_clock::now();
    //iterates through games database and assigns each one a rating based off genre compatibility and metacritic score
    for (int i = 0; i < gamesListValue.size(); i++) {
        int rating = 0;
        string name = gamesListValue[i]["name"].asString();
        for (int j = 0; j < gamesListValue[i]["genres"].size(); j++) {
            rating += genres[gamesListValue[i]["genres"][j]["name"].asString()];
        }
        if (gamesListValue[i]["metacritic"].asInt() == 0) {
            //if no metacritic exists, we give it the average metacritic rating.
            rating *= 62;
        } else {
            rating *= gamesListValue[i]["metacritic"].asInt();
        }
        orderedMap[rating] = name;
    }
    //stops clock
    end = chrono::high_resolution_clock::now();
    operationTime = chrono::duration_cast<chrono::milliseconds>(end - start);

    return orderedMap;
}

//prints top 10 highest rated games from red black tree
void GetTopTenOrderedMap(map<int, string> gameMap) {

    map<int, string>::reverse_iterator it;
    int j = 0;
    cout << "Here are 10 games we think you should try:" << endl;
    for (it = gameMap.rbegin(); it != gameMap.rend(); it++) {
        j++;
        if (j == 11) break;
        cout << j << ". " << it->second << endl;
    }

}
