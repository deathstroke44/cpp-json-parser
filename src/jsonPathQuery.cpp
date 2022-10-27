#include "library/JsonStreaming.hh"

using namespace std;

vector<Node> currentJsonPathStack;
string currentJsonPath;
map<string, string> jsonPathQueryResultsMap;
map<string, StreamToken> lastAddedTokenInResultMap;
vector<Node> jsonPathQueryTokenized;
bool multipleResultExistForThisQuery = false;
map<string, DFAState> dpDfaState;

Node topNodeInCurrentJsonPathStack();

//............................UpDating and displaying Results....................................................................

bool appendingDelimiterNeededBeforeAppendingStreamToken(const string& jsonPath, const StreamToken &streamToken) {
    StreamToken lastAddedStreamTokenInThisJsonPath = lastAddedTokenInResultMap[jsonPath];
    if (lastAddedStreamTokenInThisJsonPath.isCreatedByMe) return false;
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

void addStreamTokenInJsonPathQueryResult(const string& jsonPath, const StreamToken& streamToken) {
    string jsonPathQueryResultInThisJsonPath = jsonPathQueryResultsMap[jsonPath];
    jsonPathQueryResultInThisJsonPath.append(
            appendingDelimiterNeededBeforeAppendingStreamToken(jsonPath, streamToken) ? "," : "");
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

void updateResultIfJsonPathQuerySatisfy(const StreamToken& streamToken) {
    DFAState dfsState = dpDfaState[currentJsonPath];
    string jsonPath;
    if (!dfsState.isNotCreatedByMe) {
        int dfaIndexesWhenDfaReachedFinalStatesSize = dfsState.lengthsOfCurrentJsonPathStackWhenDfaReachedAcceptState.size();
        if (dfaIndexesWhenDfaReachedFinalStatesSize == 0) return;
        int numberOfJsonPathsAddedInResult = 0;
        for (int i = 0; i < currentJsonPathStack.size(); i++) {
            if (numberOfJsonPathsAddedInResult >= dfaIndexesWhenDfaReachedFinalStatesSize) return;
            Node node = currentJsonPathStack[i];
            if (node.isKeyNode) {
                if (node.key != "$") jsonPath.push_back('.');
                jsonPath.append(node.key);
            } else {
                jsonPath.append("[" + to_string(node.index) + "]");
            }
            if (dfsState.valueOfJsonPathUptoIthIndexWillBeAddedInResult(i)) {
                numberOfJsonPathsAddedInResult++;
                addStreamTokenInJsonPathQueryResult(jsonPath, streamToken);
            }
        }
    }
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

//........................................Initiate state of current json path's dfa from previous path dfa an update dfa accordingly

void initiateDfaOfCurrentJsonPathFromDfaOfPreviousJsonPath(DFAState &dfaOfCurrentJsonPath, DFAState &dfaOfPreviousJSonPath) {
    if (!dfaOfCurrentJsonPath.automationCurrentStates.empty()) {
        for (auto itr: dfaOfCurrentJsonPath.lengthsOfCurrentJsonPathStackWhenDfaReachedAcceptState) {
            dfaOfPreviousJSonPath.lengthsOfCurrentJsonPathStackWhenDfaReachedAcceptState.insert(itr);
        }
    } else {
        dfaOfPreviousJSonPath.lengthsOfCurrentJsonPathStackWhenDfaReachedAcceptState = dfaOfCurrentJsonPath.lengthsOfCurrentJsonPathStackWhenDfaReachedAcceptState;
        dfaOfPreviousJSonPath.canClearLengthsOfCurrentJsonPathStackWhenDfaReachedAcceptState = true;
    }
}

void findReachedStatesAndFinalStatesOfNewDfaUsingPreviousDfa(DFAState &previousDfaState, DFAState &newDfaState, const Node& node) {
    int acceptStateOfDfa = jsonPathQueryTokenized.size() - 1;
    int currentProcessingIndex = currentJsonPathStack.size() - 1;
    for (auto reachedState: previousDfaState.automationCurrentStates) {
        if (reachedState >= acceptStateOfDfa) continue;
        if (reachedState + 1 <= acceptStateOfDfa) {
            if (jsonPathQueryTokenized[reachedState].zeroOrMoreKey) {
                newDfaState.updateCurrentAutomationStates(reachedState, acceptStateOfDfa, currentProcessingIndex);
            }
            if (jsonPathQueryTokenized[reachedState + 1].nodeMatched(node)) {
                newDfaState.updateCurrentAutomationStates(reachedState + 1, acceptStateOfDfa, currentProcessingIndex);
            }
            if (reachedState + 2 <= acceptStateOfDfa && jsonPathQueryTokenized[reachedState + 1].zeroOrMoreKey
                    && jsonPathQueryTokenized[reachedState + 2].nodeMatched(node)) {
                newDfaState.updateCurrentAutomationStates(reachedState + 2, acceptStateOfDfa, currentProcessingIndex);
            }
        }
    }
}

void createDfaOfCurrentJsonPathFromDfaOfPreviousJsonPath(const string& previousJsonPath) {
    DFAState dfaStateOfPreviousJsonPath = dpDfaState[previousJsonPath];
    if (!dfaStateOfPreviousJsonPath.isNotCreatedByMe) {
        DFAState dfaOfCurrentJsonPath(currentJsonPath);
        initiateDfaOfCurrentJsonPathFromDfaOfPreviousJsonPath(dfaStateOfPreviousJsonPath, dfaOfCurrentJsonPath);
        Node newNodeAddedInJsonPath = topNodeInCurrentJsonPathStack();
        if (!dfaStateOfPreviousJsonPath.automationCurrentStates.empty())
            findReachedStatesAndFinalStatesOfNewDfaUsingPreviousDfa(dfaStateOfPreviousJsonPath, dfaOfCurrentJsonPath,
                                                                    newNodeAddedInJsonPath);
        dpDfaState[currentJsonPath] = dfaOfCurrentJsonPath;
    }
}

//..........................Operations in current Json path stacks

Node topNodeInCurrentJsonPathStack() { return currentJsonPathStack[currentJsonPathStack.size() - 1]; }


bool isLastNodeOfCurrentJsonPathIsKeyNode() {
    return !currentJsonPathStack.empty() && topNodeInCurrentJsonPathStack().isKeyNode;
}

bool isLastNodeOfCurrentJsonPathIsIndexNode() {
    return !currentJsonPathStack.empty() && !topNodeInCurrentJsonPathStack().isKeyNode;
}

void popNodeFromCurrentJsonPathStack() {
    DFAState dfa = dpDfaState[currentJsonPath];
    if (!currentJsonPathStack.empty()) {
        Node topNode = topNodeInCurrentJsonPathStack();
        currentJsonPathStack.pop_back();
        string removedPortionOfJsonPathString;
        removedPortionOfJsonPathString = topNode.isKeyNode ? "." + topNode.key : "[" + to_string(topNode.index) + "]";
        string previousJsonPath = "" + currentJsonPath;
        dpDfaState[previousJsonPath].clearVariablesWhichCanBeDeleted();
        dpDfaState.erase(previousJsonPath);
        currentJsonPath = currentJsonPath.substr(0, currentJsonPath.size() - removedPortionOfJsonPathString.size());
    }
}

void pushKeyNodeInCurrentJsonPathStack(const string& key) {
    string previousJsonPath = "" + currentJsonPath;
    currentJsonPathStack.emplace_back(key, true);
    currentJsonPath.append("." + key);
    createDfaOfCurrentJsonPathFromDfaOfPreviousJsonPath(previousJsonPath);
}

void pushIndexNodeInCurrentJsonPathStack(int index) {
    string previousJsonPath = "" + currentJsonPath;
    currentJsonPathStack.emplace_back(index);
    currentJsonPath.append("[" + to_string(index) + "]");
    createDfaOfCurrentJsonPathFromDfaOfPreviousJsonPath(previousJsonPath);
}

void incrementIndexOfTopNodeInCurrentJsonPathStack() {
    string removedPortionOfJsonPathString = "[" + to_string(topNodeInCurrentJsonPathStack().index) + "]";
    currentJsonPath = currentJsonPath.substr(0, currentJsonPath.size() - removedPortionOfJsonPathString.size());
    string previousJsonPath = currentJsonPath + "";
    currentJsonPathStack[currentJsonPathStack.size() - 1].index++;
    currentJsonPath.append("[" + to_string(topNodeInCurrentJsonPathStack().index) + "]");
    createDfaOfCurrentJsonPathFromDfaOfPreviousJsonPath(previousJsonPath);
}

//...........................Event handler and processing...............................................................

void processToken(const StreamToken& streamToken) {
    if (streamToken.tokenType == KEY_TOKEN) {
        updateResultIfJsonPathQuerySatisfy(streamToken);
        pushKeyNodeInCurrentJsonPathStack(streamToken.value);
    } else if (streamToken.tokenType == VALUE_TOKEN) {
        if (isLastNodeOfCurrentJsonPathIsIndexNode()) incrementIndexOfTopNodeInCurrentJsonPathStack();
        updateResultIfJsonPathQuerySatisfy(streamToken);
        if (isLastNodeOfCurrentJsonPathIsKeyNode()) popNodeFromCurrentJsonPathStack();
    } else if (streamToken.tokenType == OBJECT_STARTED_TOKEN) {
        if (isLastNodeOfCurrentJsonPathIsIndexNode()) incrementIndexOfTopNodeInCurrentJsonPathStack();
        updateResultIfJsonPathQuerySatisfy(streamToken);
    } else if (streamToken.tokenType == LIST_STARTED_TOKEN) {
        if (isLastNodeOfCurrentJsonPathIsIndexNode()) incrementIndexOfTopNodeInCurrentJsonPathStack();
        updateResultIfJsonPathQuerySatisfy(streamToken);
        pushIndexNodeInCurrentJsonPathStack(-1);
    } else if (streamToken.tokenType == LIST_ENDED_TOKEN) {
        popNodeFromCurrentJsonPathStack();
        updateResultIfJsonPathQuerySatisfy(streamToken);
        if (isLastNodeOfCurrentJsonPathIsKeyNode()) popNodeFromCurrentJsonPathStack();
    } else if (streamToken.tokenType == OBJECT_ENDED_TOKEN) {
        updateResultIfJsonPathQuerySatisfy(streamToken);
        if (isLastNodeOfCurrentJsonPathIsKeyNode()) popNodeFromCurrentJsonPathStack();
    } else if (streamToken.tokenType == DOCUMENT_END_TOKEN) printJsonPathQueryResult();
}

void handleJsonStreamParserEvent(const JsonStreamEvent<string> &jsonStreamEvent) {
    processToken(jsonStreamEvent.getStreamToken());
}

//..................................Processing Json path query..............................................

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
    DFAState dfa("$");
    dfa.automationCurrentStates.insert(0);
    if (jsonPathQuery == "$") {
        dfa.lengthsOfCurrentJsonPathStackWhenDfaReachedAcceptState.insert(0);
    } else {
        dfa.automationCurrentStates.insert(0);
    }
    dpDfaState[dfa.jsonPath] = dfa;
}

//................................................................................................................................ 

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