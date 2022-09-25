#include "library/json-streaming.hh"

using namespace std;

class KeyClass {
public:
    bool isStringKey = false, anyIndex = false, anyKey = false;
    int index;
    string key;

    KeyClass() = default;

    explicit KeyClass(int index) {
        this->index = index;
        anyIndex = (index == -2);
    };

    KeyClass(string key, bool dummy) {
        this->key = key;
        isStringKey = true;
        anyKey = (key.length() == 0 || key == "*");
    };
};


void displayJsonPathQueryResult(string &finalResult);

bool isThisKeyNotSatisfyQuery(const KeyClass &currentKey, const KeyClass &jsonPathQueryKey);

void processStreamEvent(StreamToken &streamToken, bool &ignoreEventFlag, bool &shouldAddThisEvent);

void addToJsonPathQueryResultIfNeeded(const StreamToken &streamToken, bool ignoreEventFlag, bool shouldAddThisEvent,
                                      bool previousKeyValid, string &previousKey);

vector<KeyClass> traversingPathKeysStack;
/**
 * Currently, I am processing a value token that can be an immediate part of a list/object. this stack maintains this information
  When I got a list/object end token I pop this stack
  When I got a list/object start token I push list/object value in this stack
 */
vector<string> traversingInListOrObjectStack;
/**
 * Example:
  $.*.info for this query $.bookstore.info and $.superstore.info need to be added in result
  for this case $.bookstore.info and $.superstore.info keys value will be stored in separate keys in this map
 */
map<string, string> jsonPathQueryResultsMap;
vector<string> jsonPathQueryResultKeys;
map<string, StreamToken> jsonPathQueryResultsLastAddedTokenMap;
JsonStreamEvent<string> currentEvent;
/**
 * Processed tokenized version of json path query
 */
vector<KeyClass> jsonPathQueryTokenized;
bool multiResultExist = false;

string getCurrentJsonPath() {
    string jsonPathKey;
    int jsonPathQueryLen = jsonPathQueryTokenized.size();
    int currentTraversedPathKeysStackLen = traversingPathKeysStack.size();
    for (int i = 0; i < jsonPathQueryLen && i < currentTraversedPathKeysStackLen; i++) {
        KeyClass keyClass = traversingPathKeysStack[i];
        string appendedKey = !keyClass.isStringKey ? "[" + to_string(keyClass.index) + "]" : "." + keyClass.key;
        jsonPathKey = jsonPathKey.append(appendedKey);
    }
    return jsonPathKey;
}

bool isLastKeyOfCurrentPathIndex() {
    return !traversingPathKeysStack.empty()
           && !traversingPathKeysStack[traversingPathKeysStack.size() - 1].isStringKey;
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
void addTokenInCurrentJsonPathResult(const StreamToken &streamToken, const string &currentKey,
                                     bool currentKeyNotValid = false) {
    string resultInCurrentPath = jsonPathQueryResultsMap[currentKey];
    bool isNewKey = jsonPathQueryResultsLastAddedTokenMap[currentKey].isDefault;
    resultInCurrentPath.append(appendingDelimiterNeededBefore(streamToken, currentKey) ? "," : "");
    jsonPathQueryResultsLastAddedTokenMap[currentKey] = streamToken;
    bool isTokenAdded = true;

    if (streamToken.tokenType == JsonEventType::KEY_TOKEN) {
        resultInCurrentPath += "\"" + streamToken.value + "\"" + " : ";
    }
    if (streamToken.tokenType == VALUE_TOKEN) {
        resultInCurrentPath.append(streamToken.isStringValue
                                   ? "\"" + streamToken.value + "\""
                                   : streamToken.value);
    }
    if (streamToken.tokenType == LIST_STARTED_TOKEN) {
        resultInCurrentPath.push_back('[');
    }
    if (streamToken.tokenType == LIST_ENDED_TOKEN
        && (!currentKeyNotValid || (traversingPathKeysStack.size() + 1 == jsonPathQueryTokenized.size()
                                    && jsonPathQueryTokenized[jsonPathQueryTokenized.size() - 1].isStringKey))) {
        resultInCurrentPath.push_back(']');
    } else if (streamToken.tokenType == LIST_ENDED_TOKEN) {
        isTokenAdded = false;
    }
    if (streamToken.tokenType == OBJECT_STARTED_TOKEN) {
        resultInCurrentPath.push_back('{');
    }
    if (streamToken.tokenType == OBJECT_ENDED_TOKEN) {
        resultInCurrentPath.push_back('}');
    }
    if (isTokenAdded) {
        if (isNewKey) {
            jsonPathQueryResultKeys.push_back(currentKey);
        }
        jsonPathQueryResultsMap[currentKey] = resultInCurrentPath;
    }
}

bool currentJsonPathMatchJsonPathQuery() {
    if (jsonPathQueryTokenized.size() > traversingPathKeysStack.size())
        return false;
    for (int i = 0; i < jsonPathQueryTokenized.size(); i++) {
        KeyClass currentKey = traversingPathKeysStack[i];
        KeyClass jsonPathQueryKey = jsonPathQueryTokenized[i];
        if (isThisKeyNotSatisfyQuery(currentKey, jsonPathQueryKey)) {
            return false;
        }
    }
    return true;
}

bool isThisKeyNotSatisfyQuery(const KeyClass &currentKey, const KeyClass &jsonPathQueryKey) {
    return (jsonPathQueryKey.isStringKey != currentKey.isStringKey)
           || (jsonPathQueryKey.isStringKey && !(jsonPathQueryKey.anyKey || jsonPathQueryKey.key == currentKey.key))
           || (!jsonPathQueryKey.isStringKey
               && (!((jsonPathQueryKey.anyIndex && currentKey.index != -1) ||
                     jsonPathQueryKey.index == currentKey.index)));
}

void popCurrentTraversedKeysStack() {
    if (!traversingPathKeysStack.empty()) {
        traversingPathKeysStack.pop_back();
    }
}

void popCurrentlyTraversingInListOrObjectStack() {
    if (!traversingInListOrObjectStack.empty()) {
        traversingInListOrObjectStack.pop_back();
    }
}

void IncrementIndexInTopKeyTraversedPathKeysStack() {
    if (isLastKeyOfCurrentPathIndex()) {
        traversingPathKeysStack[traversingPathKeysStack.size() - 1].index++;
    }
}

string getCurrentTokenIsPartOfObjectOrList() {
    return traversingInListOrObjectStack.empty() ? "" :
           traversingInListOrObjectStack[traversingInListOrObjectStack.size() - 1];
}

void handleNewListStarted() {
    if (getCurrentTokenIsPartOfObjectOrList() == "list" && isLastKeyOfCurrentPathIndex())
        IncrementIndexInTopKeyTraversedPathKeysStack();
    traversingPathKeysStack.emplace_back(-1);
}

void handleListEnded() {
    popCurrentlyTraversingInListOrObjectStack();
    if (isLastKeyOfCurrentPathIndex())
        popCurrentTraversedKeysStack();
    if (getCurrentTokenIsPartOfObjectOrList() == "object")
        popCurrentTraversedKeysStack();
}

void handleObjectEnded() {
    popCurrentlyTraversingInListOrObjectStack();
    if (getCurrentTokenIsPartOfObjectOrList() == "object")
        popCurrentTraversedKeysStack();
}

void handleNewValueAddedInList() {
    if (getCurrentTokenIsPartOfObjectOrList() == "list" && isLastKeyOfCurrentPathIndex()) {
        IncrementIndexInTopKeyTraversedPathKeysStack();
    }
}

void setCurrentlyTraversingListOrObject(const string &value) {
    traversingInListOrObjectStack.push_back(value);
}

void handleJsonStreamParserEvent(const JsonStreamEvent<string> &jsonStreamEvent) {
    currentEvent = jsonStreamEvent;
    StreamToken streamToken = jsonStreamEvent.getStreamToken();
    bool ignoreEventFlag = false;
    bool shouldAddThisEvent = false;
    bool previousKeyValid = currentJsonPathMatchJsonPathQuery();
    string previousKey = getCurrentJsonPath();
    string finalResult;

    processStreamEvent(streamToken, ignoreEventFlag, shouldAddThisEvent);
    addToJsonPathQueryResultIfNeeded(streamToken, ignoreEventFlag, shouldAddThisEvent, previousKeyValid, previousKey);
    if (streamToken.tokenType == DOCUMENT_END_TOKEN) {
        displayJsonPathQueryResult(finalResult);
    }
}

void addToJsonPathQueryResultIfNeeded(const StreamToken &streamToken, bool ignoreEventFlag, bool shouldAddThisEvent,
                                      bool previousKeyValid, string &previousKey) {
    bool currentKeyMatched = currentJsonPathMatchJsonPathQuery();
    if (currentKeyMatched && !ignoreEventFlag) {
        string currentKey = getCurrentJsonPath();
        addTokenInCurrentJsonPathResult(streamToken, currentKey + "");
    } else if (!currentKeyMatched && shouldAddThisEvent && previousKeyValid) {
        addTokenInCurrentJsonPathResult(streamToken, previousKey, !currentKeyMatched);
    }
}

void processStreamEvent(StreamToken &streamToken, bool &ignoreEventFlag, bool &shouldAddThisEvent) {
    if (streamToken.tokenType == KEY_TOKEN) {
        ignoreEventFlag = ignoreEventFlag || !currentJsonPathMatchJsonPathQuery();
        traversingPathKeysStack.emplace_back(streamToken.value, true);
    } else if (streamToken.tokenType == VALUE_TOKEN) {
        if (getCurrentTokenIsPartOfObjectOrList() == "object") {
            popCurrentTraversedKeysStack();
            shouldAddThisEvent = true;
        } else if (getCurrentTokenIsPartOfObjectOrList() == "list") {
            handleNewValueAddedInList();
        }
    } else if (streamToken.tokenType == LIST_STARTED_TOKEN) {
        handleNewListStarted();
        setCurrentlyTraversingListOrObject("list");
    } else if (streamToken.tokenType == LIST_ENDED_TOKEN) {
        handleListEnded();
        shouldAddThisEvent = true;
    } else if (streamToken.tokenType == OBJECT_STARTED_TOKEN) {
        if (getCurrentTokenIsPartOfObjectOrList() == "list") {
            handleNewValueAddedInList();
        }
        setCurrentlyTraversingListOrObject("object");
    } else if (streamToken.tokenType == OBJECT_ENDED_TOKEN) {
        handleObjectEnded();
        shouldAddThisEvent = true;
    }
}

void displayJsonPathQueryResult(string &finalResult) {
    for (const auto &jsonPath: jsonPathQueryResultKeys) {
        string jsonPathValue = jsonPathQueryResultsMap[jsonPath];
//        cout << "All answers: " << jsonPath << " -> " << jsonPathValue << endl;
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
}

void addKeyToJsonPathQueryProcessedList(bool listIndex, string val) {
    if (listIndex) {
        KeyClass keyClass(val == "*" ? -2 : stoi(val));
        jsonPathQueryTokenized.push_back(keyClass);
    } else {
        KeyClass keyClass(val, true);
        jsonPathQueryTokenized.push_back(keyClass);
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

int main(int argc, char **argv) {
    freopen("output.txt", "a+", stdout);
    JsonStreamParser jsonStreamParser = JsonStreamParser();
    string fileName(argv[1]);
    string jsonPathQuery(argv[2]);
    cout << "JSON path query:" << jsonPathQuery << " -filename: " << fileName << endl;
    fileName = "tests/Json files/" + fileName;
    KeyClass keyClass("$", true);
    traversingPathKeysStack.push_back(keyClass);
    processJsonPathQuery(jsonPathQuery);
    jsonStreamParser.setEventHandler(handleJsonStreamParserEvent);
    jsonStreamParser.startJsonStreaming(fileName);
}