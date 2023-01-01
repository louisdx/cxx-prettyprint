#include "printer.hpp"
#include <vector>
#include <unordered_map>
#include <map>
#include <iostream>
#include <string>


using namespace std;

int main() {
    // case1 : vector<int>
    vector<int> vec1 {0, 1, 2, 3, 4};
    cout << ToString(vec1) << endl;

    // // case2 : vector<vector<string>>
    // vector<vector<string>> vec2 {{"aa", "bb"}, {"-1+2"}, {"d", "."}};
    // cout << ToString(vec2) << endl;

    // // case3 : map<int, string>
    // map<int, string> mp1 {{42 , "answer"}, {1024 , "2^10"}};
    // cout << ToString(mp1) << endl;
}