#include "library/JsonStreaming.hh"

using namespace std;


string getJsonPathQueryResult();

bool isThisKeyNotSatisfyQuery(const JsonPathKey &currentKey, const JsonPathKey &jsonPathQueryKey);

void processStreamEvent(StreamToken &streamToken, bool &ignoreEventFlag, bool &shouldAddThisEvent);

void addToJsonPathQueryResultIfNeeded(const StreamToken &streamToken, bool ignoreEventFlag, bool shouldAddThisEvent,
                                      bool previousKeyValid, string &previousKey);

void initStates(string &jsonPathQuery);

string getAppendingString(const JsonPathKey &jsonPathKey);

vector<JsonPathKey> currentJsonPathStack;
/**
 * Currently, I am processing a value token that can be an immediate part of a list/object. this stack maintains this information
  When I got a list/object end token I pop this stack
  When I got a list/object start token I push list/object value in this stack
 */
vector<string> traversingListOrObjectStack;
/**
 * Example:
  $.*.info for this query $.bookstore.info and $.superstore.info need to be added in result
  for this case $.bookstore.info and $.superstore.info keys value will be stored in separate keys in this map
 */
map<string, string> jsonPathQueryResultsMap;
/**
 * STL map don't keep key insertion order. This list keeps track of jsonPath insertion order
 */
vector<string> jsonPathQueryResultKeys;
map<string, StreamToken> jsonPathQueryResultsLastAddedTokenMap;
/**
 * Processed tokenized version of json path query
 */
vector<JsonPathKey> jsonPathQueryTokenized;
bool multiResultExist = false;

string getCurrentJsonPath() {
    string jsonPath;
    for (int i = 0; i < jsonPathQueryTokenized.size() && i < currentJsonPathStack.size(); i++) {
        JsonPathKey jsonPathKey = currentJsonPathStack[i];
        string appendingString = getAppendingString(jsonPathKey);
        jsonPath = jsonPath.append(appendingString);
    }
    return jsonPath;
}

string getAppendingString(const JsonPathKey &jsonPathKey) {
    return !jsonPathKey.isStringKey ? "[" + to_string(jsonPathKey.index) + "]" : "." + jsonPathKey.key;
}

bool isLastKeyOfCurrentPathIndex() {
    return !currentJsonPathStack.empty()
           && !currentJsonPathStack[currentJsonPathStack.size() - 1].isStringKey;
}

/**
 *
 * @param streamToken
 * @param currentKey
 * @return
     Example 1:
     current Token : KEY TOKEN
     current result Json : { 'name': 'Samin Yeaser'
     last added token: VALUE TOKEN
     So before adding current token to the JSON result I need to add a delimiter

     Example 2:
     current Token : KEY TOKEN
     current result Json : {
     last added token: VALUE TOKEN
     So before adding current token to the JSON result there is no need to add a delimiter
     This function determines if I need to add a delimiter or not
 */
bool appendingDelimiterNeededBefore(const StreamToken &streamToken, const string &currentKey) {
    StreamToken lastAddedStreamToken = jsonPathQueryResultsLastAddedTokenMap[currentKey];
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

/**
 *
 * @param streamToken
 * @param currentKey
 * @param currentKeyNotValid
 */
void addTokenInCurrentJsonPathResult(const StreamToken &streamToken, const string &currentKey) {
    string resultInCurrentPath = jsonPathQueryResultsMap[currentKey];
    bool isNewKey = jsonPathQueryResultsLastAddedTokenMap[currentKey].isDefault;
    resultInCurrentPath.append(appendingDelimiterNeededBefore(streamToken, currentKey) ? "," : "");
    jsonPathQueryResultsLastAddedTokenMap[currentKey] = streamToken;

    if (streamToken.tokenType == JsonTokenType::KEY_TOKEN) {
        resultInCurrentPath += "\"" + streamToken.value + "\"" + " : ";
    } else if (streamToken.tokenType == VALUE_TOKEN) {
        resultInCurrentPath.append(streamToken.isStringValue
                                   ? "\"" + streamToken.value + "\""
                                   : streamToken.value);
    } else if (streamToken.tokenType == LIST_STARTED_TOKEN) {
        resultInCurrentPath.push_back('[');
    } else if (streamToken.tokenType == LIST_ENDED_TOKEN) {
        resultInCurrentPath.push_back(']');

    } else if (streamToken.tokenType == OBJECT_STARTED_TOKEN) {
        resultInCurrentPath.push_back('{');
    } else if (streamToken.tokenType == OBJECT_ENDED_TOKEN) {
        resultInCurrentPath.push_back('}');
    }
    if (isNewKey) {
        jsonPathQueryResultKeys.push_back(currentKey);
    }
    jsonPathQueryResultsMap[currentKey] = resultInCurrentPath;
}

bool currentJsonPathMatchJsonPathQuery() {
    if (jsonPathQueryTokenized.size() > currentJsonPathStack.size())
        return false;
    for (int i = 0; i < jsonPathQueryTokenized.size(); i++) {
        JsonPathKey currentKey = currentJsonPathStack[i];
        JsonPathKey jsonPathQueryKey = jsonPathQueryTokenized[i];
        if (isThisKeyNotSatisfyQuery(currentKey, jsonPathQueryKey)) {
            return false;
        }
    }
    return true;
}

bool isThisKeyNotSatisfyQuery(const JsonPathKey &currentKey, const JsonPathKey &jsonPathQueryKey) {
    return (jsonPathQueryKey.isStringKey != currentKey.isStringKey)
           || (jsonPathQueryKey.isStringKey && !(jsonPathQueryKey.anyKey || jsonPathQueryKey.key == currentKey.key))
           || (!jsonPathQueryKey.isStringKey
               && (!((jsonPathQueryKey.anyIndex && currentKey.index != -1) ||
                     jsonPathQueryKey.index == currentKey.index)));
}

void popCurrentJsonPathStack() {
    if (!currentJsonPathStack.empty()) {
        currentJsonPathStack.pop_back();
    }
}

void popCurrentlyTraversingInListOrObjectStack() {
    if (!traversingListOrObjectStack.empty()) {
        traversingListOrObjectStack.pop_back();
    }
}

void IncrementIndexInCurrentJsonPathStack() {
    if (isLastKeyOfCurrentPathIndex()) {
        currentJsonPathStack[currentJsonPathStack.size() - 1].index++;
    }
}

string getCurrentTokenIsPartOfObjectOrList() {
    return traversingListOrObjectStack.empty() ? "" :
           traversingListOrObjectStack[traversingListOrObjectStack.size() - 1];
}

bool isCurrentTokenIsPartOfObject() {
    return getCurrentTokenIsPartOfObjectOrList() == "object";
}

bool isCurrentTokenIsPartOfList() {
    return getCurrentTokenIsPartOfObjectOrList() == "list";
}

void handleNewListStarted() {
    if (isCurrentTokenIsPartOfList() && isLastKeyOfCurrentPathIndex())
        IncrementIndexInCurrentJsonPathStack();
    currentJsonPathStack.emplace_back(-1);
}

void handleListEnded() {
    popCurrentlyTraversingInListOrObjectStack();
    if (isLastKeyOfCurrentPathIndex())
        popCurrentJsonPathStack();
    if (isCurrentTokenIsPartOfObject())
        popCurrentJsonPathStack();
}

void handleObjectEnded() {
    popCurrentlyTraversingInListOrObjectStack();
    if (isCurrentTokenIsPartOfObject())
        popCurrentJsonPathStack();
}

void handleNewValueAddedInList() {
    if (isCurrentTokenIsPartOfList() && isLastKeyOfCurrentPathIndex()) {
        IncrementIndexInCurrentJsonPathStack();
    }
}

void setCurrentlyTraversingListOrObject(const string &value) {
    traversingListOrObjectStack.push_back(value);
}

void handleJsonStreamParserEvent(const JsonStreamEvent<string> &jsonStreamEvent) {
    StreamToken streamToken = jsonStreamEvent.getStreamToken();

    if (streamToken.tokenType == DOCUMENT_END_TOKEN) {
        getJsonPathQueryResult();
        return;
    }

    bool ignoreEventFlag = false;
    bool shouldAddThisEvent = false;
    bool previousKeyValid = currentJsonPathMatchJsonPathQuery();
    string previousKey = getCurrentJsonPath();

    processStreamEvent(streamToken, ignoreEventFlag, shouldAddThisEvent);
    addToJsonPathQueryResultIfNeeded(streamToken, ignoreEventFlag, shouldAddThisEvent, previousKeyValid, previousKey);
}

void addToJsonPathQueryResultIfNeeded(const StreamToken &streamToken, bool ignoreEventFlag, bool shouldAddThisEvent,
                                      bool previousKeyValid, string &previousKey) {
    bool currentKeyMatched = currentJsonPathMatchJsonPathQuery();
    if (currentKeyMatched && !ignoreEventFlag) {
        string currentKey = getCurrentJsonPath();
        addTokenInCurrentJsonPathResult(streamToken, currentKey + "");
    } else if (!currentKeyMatched && shouldAddThisEvent && previousKeyValid) {
        addTokenInCurrentJsonPathResult(streamToken, previousKey);
    }
}

void processStreamEvent(StreamToken &streamToken, bool &ignoreEventFlag, bool &shouldAddThisEvent) {
    if (streamToken.tokenType == KEY_TOKEN) {
        ignoreEventFlag = ignoreEventFlag || !currentJsonPathMatchJsonPathQuery();
        currentJsonPathStack.emplace_back(streamToken.value, true);
    } else if (streamToken.tokenType == VALUE_TOKEN) {
        if (isCurrentTokenIsPartOfObject()) {
            shouldAddThisEvent = currentJsonPathMatchJsonPathQuery();
            popCurrentJsonPathStack();
            shouldAddThisEvent = true;
        } else if (isCurrentTokenIsPartOfList()) {
            handleNewValueAddedInList();
        }
    } else if (streamToken.tokenType == LIST_STARTED_TOKEN) {
        handleNewListStarted();
        setCurrentlyTraversingListOrObject("list");
    } else if (streamToken.tokenType == LIST_ENDED_TOKEN) {
        popCurrentlyTraversingInListOrObjectStack();
        if (isLastKeyOfCurrentPathIndex())
            popCurrentJsonPathStack();
        if (isCurrentTokenIsPartOfObject()) {
            shouldAddThisEvent = currentJsonPathMatchJsonPathQuery();
            popCurrentJsonPathStack();
        }
    } else if (streamToken.tokenType == OBJECT_STARTED_TOKEN) {
        if (isCurrentTokenIsPartOfList()) {
            handleNewValueAddedInList();
        }
        setCurrentlyTraversingListOrObject("object");
    } else if (streamToken.tokenType == OBJECT_ENDED_TOKEN) {
        shouldAddThisEvent = currentJsonPathMatchJsonPathQuery();
        handleObjectEnded();
    }
}

string getJsonPathQueryResult() {
    string finalResult;
    for (const auto &jsonPath: jsonPathQueryResultKeys) {
        string jsonPathValue = jsonPathQueryResultsMap[jsonPath];
        cout << "All answers: " << jsonPath << " -> " << jsonPathValue << endl;
        if (finalResult.length()) {
            finalResult.push_back(',');
        }
        finalResult.append(jsonPathValue);
    }
    if (multiResultExist) {
        finalResult = "[" + finalResult + "]";
    }
    cout << "Got value of desired key final result: " << endl
         << finalResult << endl;
//     if (finalResult.length()) {
//       cout<<finalResult<<","<<endl;
//     }
    return finalResult;
}

void addKeyToJsonPathQueryProcessedList(bool listIndex, string val) {
    if (listIndex) {
        JsonPathKey jsonPathKey(val == "*" ? -2 : stoi(val));
        jsonPathQueryTokenized.push_back(jsonPathKey);
    } else {
        JsonPathKey jsonPathKey(val, true);
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
        multiResultExist = multiResultExist || (i.anyKey || i.anyIndex);
    }
}

void initStates(string &jsonPathQuery) {
    JsonPathKey jsonPathKey("$", true);
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