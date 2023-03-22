#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <string>
#include <set>
#include <queue>
#include <map>
#include <unordered_map>
#include <bits/stdc++.h>
typedef long long ll;
using namespace std;


unordered_map<int, int> path;
constexpr bool enableLog = false;


void diff(string &startString, string &finalString, int startFrom, int startTo, int finalFrom, int finalTo)
{
    if (startFrom > startTo || finalFrom > finalTo) {return;}

    // Create target position and check if it is already found
    int middle = (finalTo - finalFrom + 1) / 2;
    if (path.find(middle + finalFrom) != path.end()) {return;}

    // Init dp
    int dp[2][(startTo - startFrom) + 1];
    memset(dp, 0, sizeof dp);
    int middleRowPosition[2][(startTo - startFrom) + 1];
    memset(middleRowPosition, 0, sizeof middleRowPosition);

    // Fill the first row
    for (int j = 0; j <= (startTo - startFrom); ++j)
    {
        dp[0][j] = j;
        middleRowPosition[0][j] = 0;
        if (enableLog) {cout << dp[0][j] << " ";} //*
    }
    if (enableLog) {cout << endl;} //*

    // Main dp
    for (int i = 1; i <= (finalTo - finalFrom); ++i)
    {
        for (int j = 0; j <= (startTo - startFrom); ++j)
        {
            if (i == middle) {middleRowPosition[i % 2][j] = j;}

            if (j == 0)
            {
                dp[i % 2][j] = i;
            }
            else if (startString[startFrom + j - 1] == finalString[finalFrom + i - 1])
            {
                dp[i % 2][j] = dp[(i - 1) % 2][j - 1];
                if (i - 1 >= middle) {middleRowPosition[i % 2][j] = middleRowPosition[(i - 1) % 2][j - 1];}
            }
            else if (dp[(i - 1) % 2][j] > dp[i % 2][j - 1])
            {
                dp[i % 2][j] = dp[i % 2][j - 1] + 1;
                if (i >= middle) {middleRowPosition[i % 2][j] = middleRowPosition[i % 2][j - 1];}
            }
            else
            {
                dp[i % 2][j] = dp[(i - 1) % 2][j] + 1;
                if (i - 1 >= middle) {middleRowPosition[i % 2][j] = middleRowPosition[(i - 1) % 2][j];}
            }
            if (enableLog) {cout << dp[i % 2][j] << " ";} //*
        }
        if (enableLog) {cout << endl;} //*
    }
    if (enableLog) {for(int i = 0; i < startTo - startFrom + 1; ++i) {cout << middleRowPosition[(finalFrom - finalTo) % 2][i] << " ";} cout << endl;} //*

    // Print length of result
    if (startString.length() == (startTo - startFrom) && finalString.length() == (finalTo - finalFrom))
    {
        cout << dp[(finalTo - finalFrom) % 2][startTo - startFrom] << endl;
    }

    // Data to restore path
    path[middle + finalFrom] = middleRowPosition[(finalTo - finalFrom) % 2][startTo - startFrom] + startFrom;

    // Divide and conquer
    diff(startString, finalString, startFrom, path[middle + finalFrom], finalFrom, middle + finalFrom);
    diff(startString, finalString, path[middle + finalFrom], startTo, middle + finalFrom, finalTo);
}


int main()
{
    ios::sync_with_stdio(false);
    cin.tie(0);
    
    // Load input
    int startLength, finalLength;
    string startString, finalString;
    cin >> startLength >> startString >> finalLength >> finalString;

    // Create shortest diff
    diff(startString, finalString, 0, startLength, 0, finalLength);

    // Rest of the code is (ugly) way of formatting the result (TODO: repair this)
    string temp_ans = "";
    bool add_char = true;
    int startStringPos = 0;
    int finalStringPos = 0;
    for (int i = 1; i <= finalLength; ++i)
    {
        if (path[i] == path[i-1])
        {
            temp_ans = temp_ans + "{+" + finalString[finalStringPos] + "}";
            finalStringPos++;
        } 
        else 
        {
            add_char = true;
            while (startStringPos < path[i]) 
            {
                if (startString[startStringPos] == finalString[finalStringPos] && startStringPos == path[i] - 1)
                {
                    temp_ans = temp_ans + startString[startStringPos];
                    startStringPos++;
                    add_char = false;
                }
                else {
                    temp_ans = temp_ans + "{-" + startString[startStringPos] + "}";
                    startStringPos++;
                }
            }
            if (add_char)
            {
                temp_ans = temp_ans + "{+" + finalString[finalStringPos] + "}";
            }
            finalStringPos++;
        }
    }


    string ans = "";
    
    string add = "";
    string remove = "";
    int phase = 0;

    for (int i = 0; i < temp_ans.length(); i++) {
        if (temp_ans[i] == '{') {
            phase = 1;
        } else if (temp_ans[i] == '}') {
            phase = 0;
        } else if (phase == 1) {
            if (temp_ans[i] == '+') {
                phase = 2;
            } else if (temp_ans[i] == '-') {
                phase = 3;
            }
        } else if (phase == 2) {
            add += temp_ans[i];
        } else if (phase == 3) {
            remove += temp_ans[i];
        } else {
            if (add.length() > 0) {
                ans += "{+" + add + "}";
            }
            if (remove.length() > 0) {
                ans += "{-" + remove + "}";
            }
            add = "";
            remove = "";
            ans += temp_ans[i];
        }
        
    }
    
    cout << ans << endl;

    return 0;
}