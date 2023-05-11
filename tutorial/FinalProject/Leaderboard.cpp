//
// Created by Ofir Gilad on 13/02/2023.
//

#include "Leaderboard.h"


void Leaderboard::InitLeaderboard() {
    std::ifstream file("jsons/leaderboard.json");

    if (file.good()) {
        std::cout << "Leaderboard exists! \nReading data" << std::endl;
        leaderboard_available = true;
        // Read savedata
        file >> leaderboard_data;

        // Leaderboard data
        leaderboard_list.clear();

        for (int i = 0; i < 10; i++) {
            string index = to_string(i);
            string name = leaderboard_data[index]["name"];
            int score = leaderboard_data[index]["score"];
            leaderboard_list.push_back({ name, score });
        }
    }
    else {
        std::cout << "Leaderboard does not exist!" << std::endl;
        leaderboard_available = false;
    }
}

void Leaderboard::ResetLeaderboard() {
    if (leaderboard_available) {
        leaderboard_data = json();
    }

    // Leaderboard data
    for (int i = 0; i < 10; i++) {
        string index = to_string(i);
        leaderboard_data[index] = {};

        string name = "AAA";
        for (int j = 0; j < 3; j++) {
            name[j] += i;
        }

        int score = (10 - i) * 100;
        leaderboard_list.push_back({ name, score });
        leaderboard_data[index]["name"] = name;
        leaderboard_data[index]["score"] = score;
    }

    std::ofstream outfile("jsons/leaderboard.json");
    outfile << leaderboard_data.dump(4) << std::endl;
}

void Leaderboard::SaveLeaderboard() {
    // Leaderboard data
    for (int i = 0; i < 10; i++) {
        string index = to_string(i);

        leaderboard_data[index]["name"] = leaderboard_list[i].first;
        leaderboard_data[index]["score"] = leaderboard_list[i].second;
    }

    std::ofstream outfile("jsons/leaderboard.json");
    outfile << leaderboard_data.dump(4) << std::endl;
}

int Leaderboard::CalculateLeaderboardPosition(int score) {
    for (int i = 0; i < 10; i++) {
        if (score > leaderboard_list[i].second) {
            return i;
        }
    }
    return -1;
}

void Leaderboard::AddScoreToLeaderboard(int position, string name, int score) {
    auto leaderboard_list_iterator = leaderboard_list.begin() + position;
    // Insert element with value 9 at 4th Position in vector
    leaderboard_list.insert(leaderboard_list_iterator, { name, score });
    leaderboard_list.pop_back();
}