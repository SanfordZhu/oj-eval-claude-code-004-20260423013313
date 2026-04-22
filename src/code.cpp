#include <bits/stdc++.h>
using namespace std;

// Simplified skeleton to satisfy compilation and I/O format.
// Outputs Invalid for any non-empty command except blank lines; supports exit/quit.
// This is a placeholder to establish build, submission, and iteration.

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string line;
    while (true) {
        if(!std::getline(cin, line)) break;
        // Trim spaces
        auto l = line.find_first_not_of(' ');
        if (l == string::npos) {
            // blank line: legal, no output
            continue;
        }
        // Normalize leading/trailing spaces are allowed; minimal command parsing
        string cmd;
        {
            // extract first token
            size_t i = l;
            while (i < line.size() && line[i] != ' ') i++;
            cmd = line.substr(l, i - l);
        }
        if (cmd == "exit" || cmd == "quit") {
            break;
        }
        // For now, every other command considered invalid
        cout << "Invalid\n";
    }
    return 0;
}
