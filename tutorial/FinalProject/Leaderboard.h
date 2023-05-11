//
// Created by Ofir Gilad on 13/02/2023.
//

#ifndef ENGINEFORANIMATIONCOURSE_FINALPROJECT_LEADERBOARD_H
#define ENGINEFORANIMATIONCOURSE_FINALPROJECT_LEADERBOARD_H

#endif //ENGINEFORANIMATIONCOURSE_FINALPROJECT_LEADERBOARD_H

#include <string>
#include "json.hpp"
#include <fstream>
#include <iostream>

using namespace std;
using namespace nlohmann;

class Leaderboard
{
public:
    Leaderboard() {};

    void InitLeaderboard();
    void ResetLeaderboard();
    void SaveLeaderboard();
    int CalculateLeaderboardPosition(int score);
    void AddScoreToLeaderboard(int position, string name, int score);

    bool leaderboard_available = false;

    // Leaderboard data
    vector<pair<string, int>> leaderboard_list;

private:
    json leaderboard_data;
};