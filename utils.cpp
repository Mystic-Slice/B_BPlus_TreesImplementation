#include <bits/stdc++.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

using namespace std;

int WHITE = 7;
vector<int> COLORS = { 
    FOREGROUND_RED,
    FOREGROUND_BLUE | FOREGROUND_GREEN,
    FOREGROUND_GREEN | FOREGROUND_RED,
    FOREGROUND_RED | FOREGROUND_BLUE,
    FOREGROUND_BLUE,
    FOREGROUND_GREEN,
    FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED, 
    WHITE
};

void changeColor(int desiredColor){
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), desiredColor);
}

// Just me being lazy to write a for loop
template <typename T>
ostream& operator<<(ostream& out, const vector<T>& v) {
    for (auto& element : v){
        // if(not element) continue;
        if(element == INT_MAX) continue;
        out << element << " ";
    }
    return out;
}