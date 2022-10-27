#include "library/JsonStreaming.hh"

using namespace std;

vector<Node> currentJsonPathStack;
string currentJsonPath;
map<string, string> jsonPathQueryResultsMap;
map<string, StreamToken> lastAddedTokenInResultMap;
vector<Node> jsonPathQueryTokenized;
bool multipleResultExistForThisQuery = false;
map<string, DFA> dpDfa;

void addTokenToJsonPathQueryResultV1(const string& jsonPath, const StreamToken& streamToken);

void findReachedStatesAndFinalStatesOfNewDfaUsingPreviousDfa(DFA &previousDfaState, DFA &newDfaState, const Node& node) {
    int acceptStateOfDfa = jsonPathQueryTokenized.size() - 1;
    int currentProcessingIndex = currentJsonPathStack.size() - 1;
    for (auto reachedState: previousDfaState.dfaCurrentStates) {
        if (reachedState >= acceptStateOfDfa) continue;
        if (reachedState + 1 <= acceptStateOfDfa) {
            if (jsonPathQueryTokenized[reachedState].zeroOrMoreKey) {
                newDfaState.updateReachStates(reachedState, acceptStateOfDfa, currentProcessingIndex);
            }
            if (jsonPathQueryTokenized[reachedState + 1].nodeMatched(node)) {
                newDfaState.updateReachStates(reachedState + 1, acceptStateOfDfa, currentProcessingIndex);
            }
            if (reachedState + 2 <= acceptStateOfDfa && jsonPathQueryTokenized[reachedState + 1].zeroOrMoreKey
                    && jsonPathQueryTokenized[reachedState + 2].nodeMatched(node)) {
                newDfaState.updateReachStates(reachedState + 2, acceptStateOfDfa, currentProcessingIndex);
            }
        }
    }
}

void popNodeFromCurrentJsonPath() {
    DFA dfa = dpDfa[currentJsonPath];
    if (!currentJsonPathStack.empty()) {
        Node topNode = currentJsonPathStack[currentJsonPathStack.size() - 1];
        currentJsonPathStack.pop_back();
        string removedPortionOfJsonPathString;
        removedPortionOfJsonPathString = topNode.isKey ? "." + topNode.key : "[" + to_string(topNode.index) + "]";
        string previousJsonPath = "" + currentJsonPath;
        dpDfa[previousJsonPath].clearVariablesWhichCanBeDeleted();
        dpDfa.erase(previousJsonPath);
        currentJsonPath = currentJsonPath.substr(0, currentJsonPath.size() - removedPortionOfJsonPathString.size());
    }
}

void initiateDfaOfCurrentJsonPathFromDfaOfPreviousJsonPath(DFA &dfaOfCurrentJsonPath, DFA &dfaOfPreviousJSonPath) {
    if (!dfaOfCurrentJsonPath.dfaCurrentStates.empty()) {
        for (auto itr: dfaOfCurrentJsonPath.indexesInCurrentJsonPathStackWhenDfaReachedAcceptStates) {
            dfaOfPreviousJSonPath.indexesInCurrentJsonPathStackWhenDfaReachedAcceptStates.insert(itr);
        }
    } else {
        dfaOfPreviousJSonPath.indexesInCurrentJsonPathStackWhenDfaReachedAcceptStates = dfaOfCurrentJsonPath.indexesInCurrentJsonPathStackWhenDfaReachedAcceptStates;
        dfaOfPreviousJSonPath.indexesInCurrentJsonPathStackWhenDfaReachedAcceptStatesCanNotBeCleared = true;
    }
}

void updateResultIfJsonPathQuerySatisfy(const StreamToken& streamToken) {
    DFA dfsState = dpDfa[currentJsonPath];
    string jsonPath;
    if (!dfsState.isDefault) {
        int dfaIndexesWhenDfaReachedFinalStatesSize = dfsState.indexesInCurrentJsonPathStackWhenDfaReachedAcceptStates.size();
        if (dfaIndexesWhenDfaReachedFinalStatesSize == 0) return;
        int numberOfJsonPathsAddedInResult = 0;
        for (int i = 0; i < currentJsonPathStack.size(); i++) {
            if (numberOfJsonPathsAddedInResult >= dfaIndexesWhenDfaReachedFinalStatesSize) return;
            Node node = currentJsonPathStack[i];
            if (node.isKey) {
                if (node.key != "$") jsonPath.push_back('.');
                jsonPath.append(node.key);
            } else {
                jsonPath.append("[" + to_string(node.index) + "]");
            }
            if (dfsState.indexesInCurrentJsonPathStackWhenDfaReachedAcceptStates.find(i) !=
                dfsState.indexesInCurrentJsonPathStackWhenDfaReachedAcceptStates.end()) {
                numberOfJsonPathsAddedInResult++;
                addTokenToJsonPathQueryResultV1(jsonPath, streamToken);
            }
        }
    }
}

Node topNodeInCurrentJsonPathStack() { return currentJsonPathStack[currentJsonPathStack.size() - 1]; }

void updateDfa(const string& previousJsonPath) {
    DFA dfsOfPreviousJsonPath = dpDfa[previousJsonPath];
    if (!dfsOfPreviousJsonPath.isDefault) {
        DFA dfaOfCurrentJsonPath(currentJsonPath);
        initiateDfaOfCurrentJsonPathFromDfaOfPreviousJsonPath(dfsOfPreviousJsonPath, dfaOfCurrentJsonPath);
        Node newNodeAddedInJsonPath = topNodeInCurrentJsonPathStack();
        if (!dfsOfPreviousJsonPath.dfaCurrentStates.empty())
            findReachedStatesAndFinalStatesOfNewDfaUsingPreviousDfa(dfsOfPreviousJsonPath, dfaOfCurrentJsonPath,
                                                                    newNodeAddedInJsonPath);
        dpDfa[currentJsonPath] = dfaOfCurrentJsonPath;
    }
}


void pushKeyInCurrentJsonPath(const string& key) {
    string previousJsonPath = "" + currentJsonPath;
    currentJsonPathStack.emplace_back(key, true);
    currentJsonPath.append("." + key);
    updateDfa(previousJsonPath);
}

void pushIndexInCurrentJsonPath(int index) {
    string previousJsonPath = "" + currentJsonPath;
    currentJsonPathStack.emplace_back(index);
    currentJsonPath.append("[" + to_string(index) + "]");
    updateDfa(previousJsonPath);
}

void incrementLastNodeIndexInCurrentJsonPath() {
    string removedPortionOfJsonPathString = "[" + to_string(topNodeInCurrentJsonPathStack().index) + "]";
    currentJsonPath = currentJsonPath.substr(0, currentJsonPath.size() - removedPortionOfJsonPathString.size());
    string previousJsonPath = currentJsonPath + "";
    currentJsonPathStack[currentJsonPathStack.size() - 1].index++;
    currentJsonPath.append("[" + to_string(topNodeInCurrentJsonPathStack().index) + "]");
    updateDfa(previousJsonPath);
}

bool isLastNodeOfCurrentJsonPathIsKey() {
    return !currentJsonPathStack.empty() && topNodeInCurrentJsonPathStack().isKey;
}

bool isLastNodeOfCurrentJsonPathIsIndex() {
    return !currentJsonPathStack.empty() && !topNodeInCurrentJsonPathStack().isKey;
}

bool appendingDelimiterNeededBeforeAppendingThisTokenV1(const string& jsonPath, const StreamToken &streamToken) {
    StreamToken lastAddedStreamTokenInThisJsonPath = lastAddedTokenInResultMap[jsonPath];
    if (lastAddedStreamTokenInThisJsonPath.isDefault) return false;
    if (streamToken.tokenType == LIST_ENDED_TOKEN || streamToken.tokenType == OBJECT_ENDED_TOKEN) return false;
    if (streamToken.tokenType == VALUE_TOKEN || streamToken.tokenType == LIST_STARTED_TOKEN ||
        streamToken.tokenType == OBJECT_STARTED_TOKEN) {
        return lastAddedStreamTokenInThisJsonPath.tokenType == VALUE_TOKEN ||
               lastAddedStreamTokenInThisJsonPath.tokenType == OBJECT_ENDED_TOKEN ||
               lastAddedStreamTokenInThisJsonPath.tokenType == LIST_ENDED_TOKEN;
    }
    if (streamToken.tokenType == KEY_TOKEN) return lastAddedStreamTokenInThisJsonPath.tokenType != OBJECT_STARTED_TOKEN;
    return false;
}

void addTokenToJsonPathQueryResultV1(const string& jsonPath, const StreamToken& streamToken) {
    string jsonPathQueryResultInThisJsonPath = jsonPathQueryResultsMap[jsonPath];
    jsonPathQueryResultInThisJsonPath.append(
            appendingDelimiterNeededBeforeAppendingThisTokenV1(jsonPath, streamToken) ? "," : "");
    lastAddedTokenInResultMap[jsonPath] = streamToken;
    if (streamToken.tokenType == JsonTokenType::KEY_TOKEN) {
        jsonPathQueryResultInThisJsonPath += "\"" + streamToken.value + "\"" + " : ";
    } else if (streamToken.tokenType == VALUE_TOKEN) {
        jsonPathQueryResultInThisJsonPath.append(
                streamToken.isStringValue ? "\"" + streamToken.value + "\"" : streamToken.value);
    } else if (streamToken.tokenType == LIST_STARTED_TOKEN) {
        jsonPathQueryResultInThisJsonPath.push_back('[');
    } else if (streamToken.tokenType == LIST_ENDED_TOKEN) {
        jsonPathQueryResultInThisJsonPath.push_back(']');
    } else if (streamToken.tokenType == OBJECT_STARTED_TOKEN) {
        jsonPathQueryResultInThisJsonPath.push_back('{');
    } else if (streamToken.tokenType == OBJECT_ENDED_TOKEN) {
        jsonPathQueryResultInThisJsonPath.push_back('}');
    }
    jsonPathQueryResultsMap[jsonPath] = jsonPathQueryResultInThisJsonPath;
}

void printJsonPathQueryResult() {
    string finalResult;
    for (const auto& result: jsonPathQueryResultsMap) {
        if (finalResult.length()) finalResult.push_back(',');
        finalResult.append(result.second);
    }
    if (multipleResultExistForThisQuery) finalResult = "[" + finalResult + "]";
    cout << "Got value of desired key final result: " << endl
         << finalResult << endl;
    for (const auto& result: jsonPathQueryResultsMap) {
        cout << "All answers: " << result.first << " -> " << result.second << endl;;
    }
}

void processToken(const StreamToken& streamToken) {
    if (streamToken.tokenType == KEY_TOKEN) {
        updateResultIfJsonPathQuerySatisfy(streamToken);
        pushKeyInCurrentJsonPath(streamToken.value);
    } else if (streamToken.tokenType == VALUE_TOKEN) {
        if (isLastNodeOfCurrentJsonPathIsIndex()) incrementLastNodeIndexInCurrentJsonPath();
        updateResultIfJsonPathQuerySatisfy(streamToken);
        if (isLastNodeOfCurrentJsonPathIsKey()) popNodeFromCurrentJsonPath();
    } else if (streamToken.tokenType == OBJECT_STARTED_TOKEN) {
        if (isLastNodeOfCurrentJsonPathIsIndex()) incrementLastNodeIndexInCurrentJsonPath();
        updateResultIfJsonPathQuerySatisfy(streamToken);
    } else if (streamToken.tokenType == LIST_STARTED_TOKEN) {
        if (isLastNodeOfCurrentJsonPathIsIndex()) incrementLastNodeIndexInCurrentJsonPath();
        updateResultIfJsonPathQuerySatisfy(streamToken);
        pushIndexInCurrentJsonPath(-1);
    } else if (streamToken.tokenType == LIST_ENDED_TOKEN) {
        popNodeFromCurrentJsonPath();
        updateResultIfJsonPathQuerySatisfy(streamToken);
        if (isLastNodeOfCurrentJsonPathIsKey()) popNodeFromCurrentJsonPath();
    } else if (streamToken.tokenType == OBJECT_ENDED_TOKEN) {
        updateResultIfJsonPathQuerySatisfy(streamToken);
        if (isLastNodeOfCurrentJsonPathIsKey()) popNodeFromCurrentJsonPath();
    } else if (streamToken.tokenType == DOCUMENT_END_TOKEN) printJsonPathQueryResult();
}

void handleJsonStreamParserEvent(const JsonStreamEvent<string> &jsonStreamEvent) {
    processToken(jsonStreamEvent.getStreamToken());
}

void addKeyToJsonPathQueryProcessedList(bool listIndex, const string& val) {
    if (listIndex) jsonPathQueryTokenized.emplace_back(val == "*" ? -2 : stoi(val));
    else jsonPathQueryTokenized.emplace_back(val, true);
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

void initJsonPathQueryStates(string &jsonPathQuery) {
    currentJsonPath = "$";
    Node jsonPathKey("$", true);
    currentJsonPathStack.push_back(jsonPathKey);
    processJsonPathQuery(jsonPathQuery);
    DFA dfa("$");
    dfa.dfaCurrentStates.insert(0);
    if (jsonPathQuery == "$") {
        dfa.indexesInCurrentJsonPathStackWhenDfaReachedAcceptStates.insert(0);
    } else {
        dfa.dfaCurrentStates.insert(0);
    }
    dpDfa[dfa.jsonPath] = dfa;
}

void executeJsonPathQuery(string fileName, string jsonPathQuery) {
    cout << "JSON path query:" << jsonPathQuery << " -filename: " << fileName << endl;
    fileName = "tests/Json files/" + fileName;
    initJsonPathQueryStates(jsonPathQuery);
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