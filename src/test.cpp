#include <iostream>
#include <regex>
#include<string.h>
using namespace std;
 
int main()
{
    // Target sequence
    // string s = "I am looking for GeeksForGeeks "
    //            "articles";
    // string s = "user[1].jobTitle[1].id";
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