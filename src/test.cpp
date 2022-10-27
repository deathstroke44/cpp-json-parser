#include <iostream>
#include <regex>
#include<string.h>
#include<bits/stdc++.h>
using namespace std;

void processJsonPathQuery(string jsonPathQuery) {
    size_t found = jsonPathQuery.find('.');
    size_t LeftBracketFound;
    size_t RightBracketFound;
    vector<int> occurrenceIndexesOfDotInQuery; 
    while (found != string::npos)
    {
        occurrenceIndexesOfDotInQuery.push_back(found);
        found = jsonPathQuery.find('.', found+1);
    }
    for(int i=0;i<occurrenceIndexesOfDotInQuery.size();i++) {
        int startIndex = occurrenceIndexesOfDotInQuery[i] + 1;
        int endIndex = jsonPathQuery.size() - 1;
        if (i+1<=occurrenceIndexesOfDotInQuery.size() -1) {
            endIndex = occurrenceIndexesOfDotInQuery[i+1] - 1;
        }
        string str1 = "";
        if(startIndex<=endIndex) {
            str1 = jsonPathQuery.substr(startIndex, endIndex - startIndex + 1);
        }
        LeftBracketFound = str1.find('[');
        int keyEndIndex = -1;
        while (LeftBracketFound != string::npos) {
            RightBracketFound = str1.find(']',LeftBracketFound + 1);
            if (RightBracketFound != string::npos) {
                if (keyEndIndex == -1) {
                    keyEndIndex = LeftBracketFound;
                    cout<<str1.substr(0, LeftBracketFound)<<endl;
                }
                cout<<str1.substr(LeftBracketFound + 1, (RightBracketFound - 1) - (LeftBracketFound + 1) + 1)<<endl;
                LeftBracketFound = str1.find('[', RightBracketFound + 1);
            }
            else break;
        }
        if (keyEndIndex == -1) {
            cout<<str1<<endl;
        }
    }
}
 
int main()
{
    // Target sequence
    // string s = "I am looking for GeeksForGeeks "
    //            "articles";
    // string s = "user[1].jobTitle[1].id";
    processJsonPathQuery("$.skills.lang[0].C[1]");
    processJsonPathQuery("$.skills.lang[0].C");
    processJsonPathQuery("$.skills.lang[*].C[1]");
    processJsonPathQuery("$..skills");
    processJsonPathQuery("$..skills.*");
    string s = ".user.user.user.user";
 
    // An object of regex for pattern to be searched
    // regex r("Geek[a-zA-Z]+");
    //$..[*].jobTitle
    // regex r("^user\\[[0-9]*\\]\\.jobTitle\\..*");
    // regex r("^user\\[[0-9]*\\]\\.jobTitle$|^user\\[[0-9]*\\]\\.jobTitle\\..*|^user\\[[0-9]*\\]\\.jobTitle\\[.*");
    regex r("^(\\..+)*\\.user");
 
    // flag type for determining the matching behavior
    // here it is for matches on 'string' objects
    smatch m;
 
    // regex_search() for searching the regex pattern
    // 'r' in the string 's'. 'm' is flag for determining
    // matching behavior.
    regex_search(s, m, r);
 
    // for each loop
    for (auto x : m)
        cout << x << " ";
 
    return 0;
}