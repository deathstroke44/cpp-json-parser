#include "library/JsonStreaming.hh"
using namespace std;

vector<Node> currentJsonPathStack;
string jsonPathQueryResults = "";
StreamToken lastAddedTokenInResult;
vector<Node> jsonPathQueryTokenized;
bool multipleResultExistForThisQuery = false;

void popNodeFromCurrentJsonPath() {
    if (!currentJsonPathStack.empty()) {
        currentJsonPathStack.pop_back();
    }
}

void pushKeyInCurrentJsonPath(string key) {
    currentJsonPathStack.emplace_back(key, true);
}

void pushIndexInCurrentJsonPath(int index) {
    currentJsonPathStack.emplace_back(index);
}

void incrementLastNodeIndexInCurrentJsonPath() {
    currentJsonPathStack[currentJsonPathStack.size()-1].index++;
}

bool isLastNodeOfCurrentJsonPathIsKey() {
    return !currentJsonPathStack.empty() && currentJsonPathStack[currentJsonPathStack.size()-1].isKey;
}

bool isLastNodeOfCurrentJsonPathIsIndex() {
    return !currentJsonPathStack.empty() && !currentJsonPathStack[currentJsonPathStack.size()-1].isKey;
}

bool appendingDelimiterNeededBeforeAppendingThisToken(const StreamToken &streamToken) {
    StreamToken lastAddedStreamToken = lastAddedTokenInResult;
    if (lastAddedStreamToken.isDefault)
        return false;
    if (streamToken.tokenType == LIST_ENDED_TOKEN || streamToken.tokenType == OBJECT_ENDED_TOKEN) {
        return false;
    }
    if (streamToken.tokenType == VALUE_TOKEN || streamToken.tokenType == LIST_STARTED_TOKEN ||
        streamToken.tokenType == OBJECT_STARTED_TOKEN) {
        return lastAddedStreamToken.tokenType == VALUE_TOKEN || lastAddedStreamToken.tokenType == OBJECT_ENDED_TOKEN ||
               lastAddedStreamToken.tokenType == LIST_ENDED_TOKEN;
    }
    if (streamToken.tokenType == KEY_TOKEN) {
        return lastAddedStreamToken.tokenType != OBJECT_STARTED_TOKEN;
    }
    return false;
}

void addTokenToJsonPathQueryResult(StreamToken streamToken) {
    jsonPathQueryResults.append(appendingDelimiterNeededBeforeAppendingThisToken(streamToken) ? "," : "");
    lastAddedTokenInResult = streamToken;

    if (streamToken.tokenType == JsonTokenType::KEY_TOKEN) {
        jsonPathQueryResults += "\"" + streamToken.value + "\"" + " : ";
    } else if (streamToken.tokenType == VALUE_TOKEN) {
        jsonPathQueryResults.append(streamToken.isStringValue
                                   ? "\"" + streamToken.value + "\""
                                   : streamToken.value);
    } else if (streamToken.tokenType == LIST_STARTED_TOKEN) {
        jsonPathQueryResults.push_back('[');
    } else if (streamToken.tokenType == LIST_ENDED_TOKEN) {
        jsonPathQueryResults.push_back(']');

    } else if (streamToken.tokenType == OBJECT_STARTED_TOKEN) {
        jsonPathQueryResults.push_back('{');
    } else if (streamToken.tokenType == OBJECT_ENDED_TOKEN) {
        jsonPathQueryResults.push_back('}');
    }
}

string getAppendingString(const Node &jsonPathKey) {
    return !jsonPathKey.isKey ? "[" + to_string(jsonPathKey.index) + "]" : "." + jsonPathKey.key;
}

string getJsonPathConsideringQuery() {
    string jsonPath;
    for (int i = 0; i < jsonPathQueryTokenized.size() && i < currentJsonPathStack.size(); i++) {
        Node jsonPathKey = currentJsonPathStack[i];
        string appendingString = getAppendingString(jsonPathKey);
        jsonPath = jsonPath.append(appendingString);
    }
    return jsonPath;
}

bool isThisKeyNotSatisfyQuery(const Node &currentKey, const Node &jsonPathQueryKey) {
    return (jsonPathQueryKey.isKey != currentKey.isKey)
           || (jsonPathQueryKey.isKey && !(jsonPathQueryKey.anyKey || jsonPathQueryKey.key == currentKey.key))
           || (!jsonPathQueryKey.isKey
               && (!((jsonPathQueryKey.anyIndex && currentKey.index != -1) ||
                     jsonPathQueryKey.index == currentKey.index)));
}

bool isCurrentPathSatisfyQuery() {
    if (jsonPathQueryTokenized.size() > currentJsonPathStack.size())
        return false;
    for (int i = 0; i < jsonPathQueryTokenized.size(); i++) {
        Node currentKey = currentJsonPathStack[i];
        Node jsonPathQueryKey = jsonPathQueryTokenized[i];
        if (isThisKeyNotSatisfyQuery(currentKey, jsonPathQueryKey)) {
            return false;
        }
    }
    return true;
}

void processToken(StreamToken streamToken) {
    if (streamToken.tokenType == KEY_TOKEN) {
        if (isCurrentPathSatisfyQuery()) {
            addTokenToJsonPathQueryResult(streamToken);
        }
        pushKeyInCurrentJsonPath(streamToken.value);
    }
    else if (streamToken.tokenType == VALUE_TOKEN) {
        if (isLastNodeOfCurrentJsonPathIsIndex()) {
            incrementLastNodeIndexInCurrentJsonPath();
        }
        if (isCurrentPathSatisfyQuery()) {
            addTokenToJsonPathQueryResult(streamToken);
        }
        if (isLastNodeOfCurrentJsonPathIsKey()) {
            popNodeFromCurrentJsonPath();
        }
    }
    else if (streamToken.tokenType == OBJECT_STARTED_TOKEN) {
        if (isLastNodeOfCurrentJsonPathIsIndex()) {
            incrementLastNodeIndexInCurrentJsonPath();
        }
        if (isCurrentPathSatisfyQuery()) {
            addTokenToJsonPathQueryResult(streamToken);
        }
    }
    else if (streamToken.tokenType == LIST_STARTED_TOKEN) {
        if (isLastNodeOfCurrentJsonPathIsIndex()) {
            incrementLastNodeIndexInCurrentJsonPath();
        }
        if (isCurrentPathSatisfyQuery()) {
            addTokenToJsonPathQueryResult(streamToken);
        }
        pushIndexInCurrentJsonPath(-1);
    }
    else if (streamToken.tokenType == LIST_ENDED_TOKEN) {
        popNodeFromCurrentJsonPath();
        if (isCurrentPathSatisfyQuery()) {
            addTokenToJsonPathQueryResult(streamToken);
        }
        if (isLastNodeOfCurrentJsonPathIsKey()) {
            popNodeFromCurrentJsonPath();
        }
    }

    else if (streamToken.tokenType == OBJECT_ENDED_TOKEN) {
        if (isCurrentPathSatisfyQuery()) {
            addTokenToJsonPathQueryResult(streamToken);
        }
        if (isLastNodeOfCurrentJsonPathIsKey()) {
            popNodeFromCurrentJsonPath();
        }
    }
    else if (streamToken.tokenType == DOCUMENT_END_TOKEN) {
        if (multipleResultExistForThisQuery) {
            jsonPathQueryResults = "[" + jsonPathQueryResults + "]";
        }
        cout << "Got value of desired key final result: " << endl
             << jsonPathQueryResults << endl;
    }
}

void handleJsonStreamParserEvent(const JsonStreamEvent<string> &jsonStreamEvent) {
    processToken(jsonStreamEvent.getStreamToken());
}

void addKeyToJsonPathQueryProcessedList(bool listIndex, string val) {
    if (listIndex) {
        Node jsonPathKey(val == "*" ? -2 : stoi(val));
        jsonPathQueryTokenized.push_back(jsonPathKey);
    } else {
        Node jsonPathKey(val, true);
        jsonPathQueryTokenized.push_back(jsonPathKey);
    }
}

void processJsonPathQuery(string jsonPathQuery) {
    string curr;
    bool listIndex = false;
    int lastListEnd = -1, lastDotIndex = -1;

    for (int i = 0; i < jsonPathQuery.size(); i++) {
        if (jsonPathQuery[i] == '.') {
            if (i - 1 != lastListEnd) {
                addKeyToJsonPathQueryProcessedList(listIndex, curr);
                lastDotIndex = i;
            }
            curr.clear();
        } else if (jsonPathQuery[i] == '[') {
            if (i - 1 != lastDotIndex && i - 1 != lastListEnd) {
                addKeyToJsonPathQueryProcessedList(listIndex, curr);
            }
            listIndex = true;
            curr.clear();
        } else if (jsonPathQuery[i] == ']') {
            addKeyToJsonPathQueryProcessedList(listIndex, curr);
            listIndex = false;
            curr.clear();
            lastListEnd = i;
        } else
            curr.push_back(jsonPathQuery[i]);
    }
    if (curr.length())
        addKeyToJsonPathQueryProcessedList(listIndex, curr);

    for (auto &i: jsonPathQueryTokenized) {
        multipleResultExistForThisQuery = multipleResultExistForThisQuery || (i.anyKey || i.anyIndex);
    }
}

void initStates(string &jsonPathQuery) {
    Node jsonPathKey("$", true);
    currentJsonPathStack.push_back(jsonPathKey);
    processJsonPathQuery(jsonPathQuery);
}

void executeJsonPathQuery(string fileName, string jsonPathQuery) {
    cout << "JSON path query:" << jsonPathQuery << " -filename: " << fileName << endl;
    fileName = "tests/Json files/" + fileName;
    initStates(jsonPathQuery);
    JsonStreamParser jsonStreamParser = JsonStreamParser();
    jsonStreamParser.setEventHandler(handleJsonStreamParserEvent);
    jsonStreamParser.startJsonStreaming(fileName);
}

int main(int argc, char **argv) {
    freopen("output.txt", "a+", stdout);
    JsonStreamParser jsonStreamParser = JsonStreamParser();
    string fileName(argv[1]);
    string jsonPathQuery(argv[2]);
    executeJsonPathQuery(fileName, jsonPathQuery);
}