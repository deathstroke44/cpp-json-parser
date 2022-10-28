#include "library/JsonStreaming.hh"

using namespace std;

vector<Node> currentJsonPathList;
map<string, string> jsonPathQueryResultsMap;
map<string, StreamToken> lastAddedTokenInResultMap;
vector<Node> jsonPathQueryTokenized;
bool multipleResultExistForThisQuery = false;
map<string, DFAState> dpDfaState;
int countOfReachingAcceptStateInCurrentPath = 0;

Node getLastNodeOfCurrentJsonPath();

bool
appendingDelimiterNeededBeforeAppendingStreamTokenInResult(const string &jsonPath, const StreamToken &streamToken) {
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
            appendingDelimiterNeededBeforeAppendingStreamTokenInResult(jsonPath, streamToken) ? "," : "");
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

void updateResultIfJsonPathQuerySatisfy(StreamToken streamToken) {
    if (countOfReachingAcceptStateInCurrentPath) {
        string jsonPath;
        int numberOfJsonPathsAddedInResult = 0;
        for (int i = 0; i < currentJsonPathList.size(); i++) {
            if (numberOfJsonPathsAddedInResult >= countOfReachingAcceptStateInCurrentPath) return;
            Node node = currentJsonPathList[i];
            if (node.isKeyNode) {
                jsonPath.append(node.key != "$" ? "." + node.key : node.key);
            } else {
                jsonPath.append("[" + to_string(node.index) + "]");
            }
            if (node.reachAcceptStates) {
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

void createDfaStateOfChildNodeFromDfaOfParentNode(int indexOfCurrentJsonPath) {
    Node parentNode = currentJsonPathList[indexOfCurrentJsonPath-1];
    currentJsonPathList.back().automationStates.clear();
    currentJsonPathList.back().reachAcceptStates = false;
    int acceptStateOfDfa = jsonPathQueryTokenized.size() - 1;
    int currentProcessingIndex = currentJsonPathList.size() - 1;
    for (auto reachedState: parentNode.automationStates) {
        if (reachedState >= acceptStateOfDfa) continue;
        if (reachedState + 1 <= acceptStateOfDfa) {
            if (jsonPathQueryTokenized[reachedState].recursiveDescent) {
                currentJsonPathList.back().updateCurrentAutomationStates(reachedState, acceptStateOfDfa, currentProcessingIndex);
            }
            if (jsonPathQueryTokenized[reachedState + 1].nodeMatched(currentJsonPathList.back())) {
                currentJsonPathList.back().updateCurrentAutomationStates(reachedState + 1, acceptStateOfDfa, currentProcessingIndex);
            }
            if (reachedState + 2 <= acceptStateOfDfa && jsonPathQueryTokenized[reachedState + 1].recursiveDescent
                && jsonPathQueryTokenized[reachedState + 2].nodeMatched(currentJsonPathList.back())) {
                currentJsonPathList.back().updateCurrentAutomationStates(reachedState + 2, acceptStateOfDfa, currentProcessingIndex);
            }
        }
    }
    if (currentJsonPathList.back().reachAcceptStates) {
        countOfReachingAcceptStateInCurrentPath++;
    }
}

//..........................Operations in current Json path stacks

Node getLastNodeOfCurrentJsonPath() { return currentJsonPathList.back(); }


bool isLastNodeOfCurrentJsonPathIsKeyNode() {
    return !currentJsonPathList.empty() && getLastNodeOfCurrentJsonPath().isKeyNode;
}

bool isLastNodeOfCurrentJsonPathIsIndexNode() {
    return !currentJsonPathList.empty() && !getLastNodeOfCurrentJsonPath().isKeyNode;
}

void popNodeFromCurrentJsonPath() {
    if (!currentJsonPathList.empty()) {
        Node lastNode = getLastNodeOfCurrentJsonPath();
        if (lastNode.reachAcceptStates) {
            countOfReachingAcceptStateInCurrentPath--;
            lastNode.reachAcceptStates = false;
        }
        lastNode.automationStates.clear();
        currentJsonPathList.pop_back();
    }
}

void pushKeyNodeInCurrentJsonPath(const string &key) {
    currentJsonPathList.emplace_back(key, true);
    createDfaStateOfChildNodeFromDfaOfParentNode(currentJsonPathList.size() - 1);
}

void pushIndexNodeInCurrentJsonPath(int index) {
    currentJsonPathList.emplace_back(index);
}

void incrementIndexOfLastNodeInCurrentJsonPath() {
    if (currentJsonPathList.back().reachAcceptStates) {
        countOfReachingAcceptStateInCurrentPath--;
    }
    currentJsonPathList.back().automationStates.clear();
    currentJsonPathList.back().reachAcceptStates = false;
    currentJsonPathList.back().index++;
    createDfaStateOfChildNodeFromDfaOfParentNode(currentJsonPathList.size() - 1);
    if (getLastNodeOfCurrentJsonPath().reachAcceptStates) {
        countOfReachingAcceptStateInCurrentPath++;
    }
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
    Node node("$", true);
    if (jsonPathQuery == "$") {
        node.reachAcceptStates = true;
    } else {
        node.automationStates.insert(0);
    }
    currentJsonPathList.push_back(node);
    processJsonPathQuery(jsonPathQuery);
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