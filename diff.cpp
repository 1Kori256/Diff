#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <limits>
#include <stdexcept>
using namespace std;

unordered_map<int, int> path;
constexpr bool enableLog = false;


void diff(const string &startString, const string &finalString, int startFrom, int startTo, int finalFrom, int finalTo)
{
    if (startFrom > startTo || finalFrom > finalTo) {return;}

    // Create target position and check if it is already found
    int middle = (finalTo - finalFrom + 1) / 2;
    if (path.find(middle + finalFrom) != path.end()) {return;}

    // Init dp
    const int width = startTo - startFrom + 1;
    vector<int> dp[2] = {vector<int>(width), vector<int>(width)};
    vector<int> middleRowPosition[2] = {vector<int>(width), vector<int>(width)};

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
    if (enableLog) {for(int i = 0; i < startTo - startFrom + 1; ++i) {cout << middleRowPosition[(finalTo - finalFrom) % 2][i] << " ";} cout << endl;} //*

    // Print length of result
    if (startString.length() == static_cast<size_t>(startTo - startFrom) && finalString.length() == static_cast<size_t>(finalTo - finalFrom))
    {
        cout << dp[(finalTo - finalFrom) % 2][startTo - startFrom] << endl;
    }

    // Data to restore path
    path[middle + finalFrom] = middleRowPosition[(finalTo - finalFrom) % 2][startTo - startFrom] + startFrom;

    // Release the work rows before recursion to keep memory linear.
    for (auto &row : dp) {vector<int>().swap(row);}
    for (auto &row : middleRowPosition) {vector<int>().swap(row);}

    // Divide and conquer
    diff(startString, finalString, startFrom, path[middle + finalFrom], finalFrom, middle + finalFrom);
    diff(startString, finalString, path[middle + finalFrom], startTo, middle + finalFrom, finalTo);
}


// Build runs directly; input characters must never be parsed as edit markers.
string format_diff(const string &startString, const string &finalString)
{
    string result, run;
    result.reserve(startString.size() + finalString.size());
    char phase = 0;
    auto flush = [&]() {
        if (run.empty()) {return;}
        if (phase != '=') {result += '{'; result += phase;}
        for (char character : run)
        {
            switch (character)
            {
                case '\\': case '{': case '}': result += '\\'; result += character; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default: result += character;
            }
        }
        if (phase != '=') {result += '}';}
        run.clear();
    };
    auto emit = [&](char operation, char character) {
        if (operation != phase) {flush(); phase = operation;}
        run += character;
    };

    size_t startPosition = 0;
    for (size_t i = 0; i < finalString.size(); ++i)
    {
        const size_t boundary = static_cast<size_t>(path.at(static_cast<int>(i + 1)));
        if (boundary < startPosition || boundary > startString.size())
        {
            throw runtime_error("Invalid reconstructed path");
        }
        bool matched = false;
        while (startPosition < boundary)
        {
            const char character = startString[startPosition++];
            if (startPosition == boundary && character == finalString[i])
            {
                emit('=', character);
                matched = true;
            }
            else {emit('-', character);}
        }
        if (!matched) {emit('+', finalString[i]);}
    }
    while (startPosition < startString.size()) {emit('-', startString[startPosition++]);}
    flush();
    return result;
}

void read_string(istream &input, string &value)
{
    long long length;
    if (!(input >> length) || length < 0 || length > numeric_limits<int>::max())
    {
        throw invalid_argument("Expected a nonnegative string length");
    }
    value.clear();
    if (length > 0 && !(input >> value)) {throw invalid_argument("Missing string after length");}
    if (value.size() != static_cast<size_t>(length))
    {
        throw invalid_argument("Declared length does not match string length");
    }
}

void usage(ostream &output)
{
    output << "Usage: diff START FINAL\n"
              "       diff < input.txt\n\n"
              "Input format: START_LENGTH START FINAL_LENGTH FINAL\n"
              "For an empty string, write 0 and omit its token.\n";
}

int main(int argc, char **argv)
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (argc == 2 && string(argv[1]) == "--help") {usage(cout); return 0;}
    try
    {
        string startString, finalString;
        if (argc == 3)
        {
            startString = argv[1];
            finalString = argv[2];
        }
        else if (argc == 1)
        {
            read_string(cin, startString);
            read_string(cin, finalString);
        }
        else {usage(cerr); return 2;}
        if (startString.size() >= static_cast<size_t>(numeric_limits<int>::max()) ||
            finalString.size() >= static_cast<size_t>(numeric_limits<int>::max()))
        {
            throw invalid_argument("Strings are too long");
        }
        diff(startString, finalString, 0, static_cast<int>(startString.size()),
             0, static_cast<int>(finalString.size()));
        cout << format_diff(startString, finalString) << '\n';
    }
    catch (const exception &error)
    {
        cerr << "diff: " << error.what() << '\n';
        return 2;
    }
    return 0;
}
