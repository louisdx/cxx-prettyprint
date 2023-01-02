#include "printer.hpp"
#include <vector>
#include <unordered_map>
#include <map>
#include <iostream>
#include <string>
#include <deque>
#include <list>


using namespace std;

int main() {
    vector<int> vec1 {0, 1, 2, 3, 4};
    cout << ToString(vec1) << endl;

    vector<vector<string>> vec2 {{"aa", "bb"}, {"-1+2"}, {"d", "."}};
    cout << ToString(vec2) << endl;

    std::string s = "simple string to print";
    cout << ToString(s) << endl;

    auto tp = std::make_tuple(1, 2, "22", "2", 5, "21");
    cout << ToString(tp) << endl;

    std::vector<float> v2 {0.1f, 0.2f, 0.3f};
    cout << ToString(v2) << endl;

    cout << ToString("char[n]") << endl;

    map<int, string> mp1 {{42 , "answer"}, {1024 , "2^10"}};
    cout << ToString(mp1) << endl;

    map<int, std::vector<std::string>> mp2 {
        {1, {
            "11", "12", "13"
        }},
        {2, {
            "21", "22"
        }},
        {3, {
            "31"
        }}
    };
    cout << ToString(mp2) << endl;

    deque<pair<int,int>> dq1;
    dq1.push_back({1,1});
    dq1.push_front({0,0});
    cout << ToString(dq1) << endl;

    list<int> lst {2,3,4,5};
    cout << ToString(lst) << endl;

    // not support;
    // cout << ToString(nullptr) << endl;
}