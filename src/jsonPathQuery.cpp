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

bool appendingDelimiterNeededBeforeAppendingStreamTokenInResult(const string& jsonPath, const StreamToken &streamToken) {
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
            appendingDelimiterNeededBeforeAppendingStreamTokenInResult(jsonPath, streamToken) ? "," : "");
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

void initiateDfaOfCurrentJsonPathFromDfaOfPreviousJsonPath(DFAState &dfaOfPreviousJsonPath, DFAState &dfaOfCurrentJSonPath) {
    if (!dfaOfPreviousJsonPath.automationCurrentStates.empty()) {
        for (auto itr: dfaOfPreviousJsonPath.lengthsOfCurrentJsonPathStackWhenDfaReachedAcceptState) {
            dfaOfCurrentJSonPath.lengthsOfCurrentJsonPathStackWhenDfaReachedAcceptState.insert(itr);
        }
    } else {
        dfaOfCurrentJSonPath.lengthsOfCurrentJsonPathStackWhenDfaReachedAcceptState = dfaOfPreviousJsonPath.lengthsOfCurrentJsonPathStackWhenDfaReachedAcceptState;
        dfaOfCurrentJSonPath.canClearLengthsOfCurrentJsonPathStackWhenDfaReachedAcceptState = true;
    }
}

void updateAutomationStatesAndFinalStatesOfNewDfaUsingPreviousDfa(DFAState &previousDfaState, DFAState &newDfaState, const Node& node) {
    int acceptStateOfDfa = jsonPathQueryTokenized.size() - 1;
    int currentProcessingIndex = currentJsonPathStack.size() - 1;
    for (auto reachedState: previousDfaState.automationCurrentStates) {
        if (reachedState >= acceptStateOfDfa) continue;
        if (reachedState + 1 <= acceptStateOfDfa) {
            if (jsonPathQueryTokenized[reachedState].recursiveDescent) {
                newDfaState.updateCurrentAutomationStates(reachedState, acceptStateOfDfa, currentProcessingIndex);
            }
            if (jsonPathQueryTokenized[reachedState + 1].nodeMatched(node)) {
                newDfaState.updateCurrentAutomationStates(reachedState + 1, acceptStateOfDfa, currentProcessingIndex);
            }
            if (reachedState + 2 <= acceptStateOfDfa && jsonPathQueryTokenized[reachedState + 1].recursiveDescent
                    && jsonPathQueryTokenized[reachedState + 2].nodeMatched(node)) {
                newDfaState.updateCurrentAutomationStates(reachedState + 2, acceptStateOfDfa, currentProcessingIndex);
            }
        }
    }
}

void createDfaStateOfCurrentJsonPathFromDfaStateOfPreviousJsonPath(const string& previousJsonPath) {
    DFAState dfaStateOfPreviousJsonPath = dpDfaState[previousJsonPath];
    if (!dfaStateOfPreviousJsonPath.isNotCreatedByMe) {
        DFAState dfaOfCurrentJsonPath(currentJsonPath);
        initiateDfaOfCurrentJsonPathFromDfaOfPreviousJsonPath(dfaStateOfPreviousJsonPath, dfaOfCurrentJsonPath);
        Node newNodeAddedInJsonPath = topNodeInCurrentJsonPathStack();
        if (!dfaStateOfPreviousJsonPath.automationCurrentStates.empty())
            updateAutomationStatesAndFinalStatesOfNewDfaUsingPreviousDfa(dfaStateOfPreviousJsonPath, dfaOfCurrentJsonPath,
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
    createDfaStateOfCurrentJsonPathFromDfaStateOfPreviousJsonPath(previousJsonPath);
}

void pushIndexNodeInCurrentJsonPathStack(int index) {
    string previousJsonPath = "" + currentJsonPath;
    currentJsonPathStack.emplace_back(index);
    currentJsonPath.append("[" + to_string(index) + "]");
    createDfaStateOfCurrentJsonPathFromDfaStateOfPreviousJsonPath(previousJsonPath);
}

void incrementIndexOfTopNodeInCurrentJsonPathStack() {
    string removedPortionOfJsonPathString = "[" + to_string(topNodeInCurrentJsonPathStack().index) + "]";
    currentJsonPath = currentJsonPath.substr(0, currentJsonPath.size() - removedPortionOfJsonPathString.size());
    string previousJsonPath = currentJsonPath + "";
    currentJsonPathStack[currentJsonPathStack.size() - 1].index++;
    currentJsonPath.append("[" + to_string(topNodeInCurrentJsonPathStack().index) + "]");
    createDfaStateOfCurrentJsonPathFromDfaStateOfPreviousJsonPath(previousJsonPath);
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

void processJsonPathQuery(string jsonPathQuery) {
    size_t found = jsonPathQuery.find('.'),LeftBracketFound,RightBracketFound;
    vector<int> occurrenceIndexesOfDotInQuery;
    jsonPathQueryTokenized.emplace_back("$", true); 
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
                    jsonPathQueryTokenized.emplace_back(str1.substr(0, LeftBracketFound), true);
                }
                jsonPathQueryTokenized.emplace_back(str1.substr(LeftBracketFound + 1, (RightBracketFound - 1) - (LeftBracketFound + 1) + 1), false);
                LeftBracketFound = str1.find('[', RightBracketFound + 1);
            }
            else break;
        }
        if (keyEndIndex == -1) {
            jsonPathQueryTokenized.emplace_back(str1, true);
        }
    }
    for (auto &token: jsonPathQueryTokenized) {
        multipleResultExistForThisQuery = multipleResultExistForThisQuery || (token.recursiveDescent || token.wildcard);
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