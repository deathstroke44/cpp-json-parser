#include "library/JsonStreaming.hh"

using namespace std;

vector<Node> currentJsonPathList;
string currentJsonPath;
map<string, string> jsonPathQueryResultsMap;
map<string, StreamToken> lastAddedTokenInResultMap;
vector<Node> jsonPathQueryTokenized;
bool multipleResultExistForThisQuery = false;
map<string, DFAState> dpDfaState;

Node getLastNodeOfCurrentJsonPath();

//............................UpDating and displaying Results....................................................................

bool
delimiterNeededBeforeAppendingStreamTokenInResult(const string &jsonPath, const StreamToken &streamToken) {
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

void addStreamTokenInJsonPathQueryResult(const string &jsonPath, const StreamToken &streamToken) {
    string jsonPathQueryResultInThisJsonPath = jsonPathQueryResultsMap[jsonPath];
    jsonPathQueryResultInThisJsonPath.append(
            delimiterNeededBeforeAppendingStreamTokenInResult(jsonPath, streamToken) ? "," : "");
    lastAddedTokenInResultMap[jsonPath] = streamToken;
    JsonTokenType jsonTokenType = streamToken.tokenType;
    if (jsonTokenType == JsonTokenType::KEY_TOKEN) {
        jsonPathQueryResultInThisJsonPath += "\"" + streamToken.value + "\"" + " : ";
    } else if (jsonTokenType == VALUE_TOKEN) {
        jsonPathQueryResultInThisJsonPath.append(
                streamToken.isStringValue ? "\"" + streamToken.value + "\"" : streamToken.value);
    } else if (jsonTokenType == LIST_STARTED_TOKEN) {
        jsonPathQueryResultInThisJsonPath.push_back('[');
    } else if (jsonTokenType == LIST_ENDED_TOKEN) {
        jsonPathQueryResultInThisJsonPath.push_back(']');
    } else if (jsonTokenType == OBJECT_STARTED_TOKEN) {
        jsonPathQueryResultInThisJsonPath.push_back('{');
    } else if (jsonTokenType == OBJECT_ENDED_TOKEN) {
        jsonPathQueryResultInThisJsonPath.push_back('}');
    }
    jsonPathQueryResultsMap[jsonPath] = jsonPathQueryResultInThisJsonPath;
}

void updateResultIfJsonPathQuerySatisfy(const StreamToken &streamToken) {
    DFAState dfaStateOfCurrentJsonPath = dpDfaState[currentJsonPath];
    string jsonPath;
    if (!dfaStateOfCurrentJsonPath.isNotCreatedByMe) {
        int dfaIndexesWhenDfaReachedFinalStatesSize = dfaStateOfCurrentJsonPath.lengthsOfCurrentJsonPathStackWhenDfaReachedAcceptState.size();
        if (dfaIndexesWhenDfaReachedFinalStatesSize == 0) return;
        int numberOfJsonPathsAddedInResult = 0;
        for (int i = 0; i < currentJsonPathList.size(); i++) {
            if (numberOfJsonPathsAddedInResult >= dfaIndexesWhenDfaReachedFinalStatesSize) return;
            Node node = currentJsonPathList[i];
            if (node.isKeyNode) {
                jsonPath.append(node.key != "$" ? "." + node.key : node.key);
            } else {
                jsonPath.append("[" + to_string(node.index) + "]");
            }
            if (dfaStateOfCurrentJsonPath.valueOfJsonPathUptoIthIndexWillBeAddedInResult(i)) {
                numberOfJsonPathsAddedInResult++;
                addStreamTokenInJsonPathQueryResult(jsonPath, streamToken);
            }
        }
    }
}

void printJsonPathQueryResult() {
    string finalResult;
    for (const auto &result: jsonPathQueryResultsMap) {
        if (finalResult.length()) finalResult.push_back(',');
        finalResult.append(result.second);
    }
    if (multipleResultExistForThisQuery) finalResult = "[" + finalResult + "]";
    cout << "Got value of desired key final result: " << endl
         << finalResult << endl;
    for (const auto &result: jsonPathQueryResultsMap) {
        cout << "Result For this JSON query: " << result.first << " -> " << result.second << endl;;
    }
}

//........................................Initiate state of current json path's dfa from previous path dfa an update dfa accordingly

void
initiateDfaOfCurrentJsonPathFromDfaOfPreviousJsonPath(DFAState &dfaOfPreviousJsonPath, DFAState &dfaOfCurrentJSonPath) {
    if (!dfaOfPreviousJsonPath.automationCurrentStates.empty()) {
        for (auto itr: dfaOfPreviousJsonPath.lengthsOfCurrentJsonPathStackWhenDfaReachedAcceptState) {
            dfaOfCurrentJSonPath.lengthsOfCurrentJsonPathStackWhenDfaReachedAcceptState.insert(itr);
        }
    } else {
        dfaOfCurrentJSonPath.lengthsOfCurrentJsonPathStackWhenDfaReachedAcceptState = dfaOfPreviousJsonPath.lengthsOfCurrentJsonPathStackWhenDfaReachedAcceptState;
        dfaOfCurrentJSonPath.canClearLengthsOfCurrentJsonPathStackWhenDfaReachedAcceptState = true;
    }
}

void updateAutomationStatesAndFinalStatesOfNewDfaUsingPreviousDfa(DFAState &previousDfaState, DFAState &newDfaState,
                                                                  const Node &node) {
    int acceptStateOfDfa = jsonPathQueryTokenized.size() - 1;
    int currentProcessingIndex = currentJsonPathList.size() - 1;
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

void createDfaStateOfCurrentJsonPathFromDfaStateOfPreviousJsonPath(const string &previousJsonPath) {
    DFAState dfaStateOfPreviousJsonPath = dpDfaState[previousJsonPath];
    if (!dfaStateOfPreviousJsonPath.isNotCreatedByMe) {
        DFAState dfaOfCurrentJsonPath(currentJsonPath);
        initiateDfaOfCurrentJsonPathFromDfaOfPreviousJsonPath(dfaStateOfPreviousJsonPath, dfaOfCurrentJsonPath);
        Node newNodeAddedInJsonPath = getLastNodeOfCurrentJsonPath();
        if (!dfaStateOfPreviousJsonPath.automationCurrentStates.empty())
            updateAutomationStatesAndFinalStatesOfNewDfaUsingPreviousDfa(dfaStateOfPreviousJsonPath,
                                                                         dfaOfCurrentJsonPath,
                                                                         newNodeAddedInJsonPath);
        dpDfaState[currentJsonPath] = dfaOfCurrentJsonPath;
    }
}

//..........................Operations in current Json path stacks

Node getLastNodeOfCurrentJsonPath() { return currentJsonPathList[currentJsonPathList.size() - 1]; }


bool isLastNodeOfCurrentJsonPathIsKeyNode() {
    return !currentJsonPathList.empty() && getLastNodeOfCurrentJsonPath().isKeyNode;
}

bool isLastNodeOfCurrentJsonPathIsIndexNode() {
    return !currentJsonPathList.empty() && !getLastNodeOfCurrentJsonPath().isKeyNode;
}

void popNodeFromCurrentJsonPath() {
    DFAState dfa = dpDfaState[currentJsonPath];
    if (!currentJsonPathList.empty()) {
        Node lastNodeOfPreviousJsonPath = getLastNodeOfCurrentJsonPath();
        currentJsonPathList.pop_back();
        string removedPortionOfJsonPathString;
        removedPortionOfJsonPathString = lastNodeOfPreviousJsonPath.isKeyNode ? "." + lastNodeOfPreviousJsonPath.key :
                                         "[" + to_string(lastNodeOfPreviousJsonPath.index) + "]";
        string previousJsonPath = "" + currentJsonPath;
        dpDfaState[previousJsonPath].clearVariablesWhichCanBeDeleted();
        dpDfaState.erase(previousJsonPath);
        currentJsonPath = currentJsonPath.substr(0, currentJsonPath.size() - removedPortionOfJsonPathString.size());
    }
}

void pushKeyNodeInCurrentJsonPath(const string &key) {
    string previousJsonPath = "" + currentJsonPath;
    currentJsonPathList.emplace_back(key, true);
    currentJsonPath.append("." + key);
    createDfaStateOfCurrentJsonPathFromDfaStateOfPreviousJsonPath(previousJsonPath);
}

void pushIndexNodeInCurrentJsonPath(int index) {
    string previousJsonPath = "" + currentJsonPath;
    currentJsonPathList.emplace_back(index);
    currentJsonPath.append("[" + to_string(index) + "]");
    createDfaStateOfCurrentJsonPathFromDfaStateOfPreviousJsonPath(previousJsonPath);
}

void incrementIndexOfLastNodeInCurrentJsonPath() {
    string removedPortionOfJsonPathString = "[" + to_string(getLastNodeOfCurrentJsonPath().index) + "]";
    currentJsonPath = currentJsonPath.substr(0, currentJsonPath.size() - removedPortionOfJsonPathString.size());
    string previousJsonPath = currentJsonPath + "";
    currentJsonPathList[currentJsonPathList.size() - 1].index++;
    currentJsonPath.append("[" + to_string(getLastNodeOfCurrentJsonPath().index) + "]");
    createDfaStateOfCurrentJsonPathFromDfaStateOfPreviousJsonPath(previousJsonPath);
}

//...........................Event handler and processing...............................................................

void handleJsonStreamParserEvent(const JsonStreamEvent<string> &jsonStreamEvent) {
    StreamToken streamToken = jsonStreamEvent.getStreamToken();
    JsonTokenType jsonTokenType = streamToken.tokenType;
    if (jsonTokenType == KEY_TOKEN) {
        updateResultIfJsonPathQuerySatisfy(streamToken);
        pushKeyNodeInCurrentJsonPath(streamToken.value);
    } else if (jsonTokenType == VALUE_TOKEN) {
        if (isLastNodeOfCurrentJsonPathIsIndexNode()) incrementIndexOfLastNodeInCurrentJsonPath();
        updateResultIfJsonPathQuerySatisfy(streamToken);
        if (isLastNodeOfCurrentJsonPathIsKeyNode()) popNodeFromCurrentJsonPath();
    } else if (jsonTokenType == OBJECT_STARTED_TOKEN) {
        if (isLastNodeOfCurrentJsonPathIsIndexNode()) incrementIndexOfLastNodeInCurrentJsonPath();
        updateResultIfJsonPathQuerySatisfy(streamToken);
    } else if (jsonTokenType == LIST_STARTED_TOKEN) {
        if (isLastNodeOfCurrentJsonPathIsIndexNode()) incrementIndexOfLastNodeInCurrentJsonPath();
        updateResultIfJsonPathQuerySatisfy(streamToken);
        pushIndexNodeInCurrentJsonPath(-1);
    } else if (jsonTokenType == LIST_ENDED_TOKEN) {
        popNodeFromCurrentJsonPath();
        updateResultIfJsonPathQuerySatisfy(streamToken);
        if (isLastNodeOfCurrentJsonPathIsKeyNode()) popNodeFromCurrentJsonPath();
    } else if (jsonTokenType == OBJECT_ENDED_TOKEN) {
        updateResultIfJsonPathQuerySatisfy(streamToken);
        if (isLastNodeOfCurrentJsonPathIsKeyNode()) popNodeFromCurrentJsonPath();
    } else if (jsonTokenType == DOCUMENT_END_TOKEN) printJsonPathQueryResult();
}

//..................................Processing Json path query..............................................

void processJsonPathQuery(string jsonPathQuery) {
    size_t indexOfDot = jsonPathQuery.find('.'), indexOfLeftSquareBracket, indexOfRightSquareBracket;
    vector<int> occurrenceIndexesOfDotInQuery;
    jsonPathQueryTokenized.emplace_back("$", true);
    while (indexOfDot != string::npos) {
        occurrenceIndexesOfDotInQuery.push_back(indexOfDot);
        indexOfDot = jsonPathQuery.find('.', indexOfDot + 1);
    }
    for (int i = 0; i < occurrenceIndexesOfDotInQuery.size(); i++) {
        int startIndex = occurrenceIndexesOfDotInQuery[i] + 1;
        int endIndex = jsonPathQuery.size() - 1;
        if (i + 1 <= occurrenceIndexesOfDotInQuery.size() - 1) {
            endIndex = occurrenceIndexesOfDotInQuery[i + 1] - 1;
        }
        string str1;
        if (startIndex <= endIndex) {
            str1 = jsonPathQuery.substr(startIndex, endIndex - startIndex + 1);
        }
        indexOfLeftSquareBracket = str1.find('[');
        int keyEndIndex = -1;
        while (indexOfLeftSquareBracket != string::npos) {
            indexOfRightSquareBracket = str1.find(']', indexOfLeftSquareBracket + 1);
            if (indexOfRightSquareBracket != string::npos) {
                if (keyEndIndex == -1) {
                    keyEndIndex = indexOfLeftSquareBracket;
                    jsonPathQueryTokenized.emplace_back(str1.substr(0, indexOfLeftSquareBracket), true);
                }
                jsonPathQueryTokenized.emplace_back(
                        str1.substr(indexOfLeftSquareBracket + 1,
                                    (indexOfRightSquareBracket - 1) - (indexOfLeftSquareBracket + 1) + 1), false);
                indexOfLeftSquareBracket = str1.find('[', indexOfRightSquareBracket + 1);
            } else break;
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
    currentJsonPathList.push_back(jsonPathKey);
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