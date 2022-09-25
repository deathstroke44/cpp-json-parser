#include "library/code.hh"

using namespace std;

class KeyClass {
public:
    bool isStringKey = false, anyIndex = false, anyKey = false;
    int index;
    string key;

    KeyClass() = default;

    KeyClass(int index) {
        this->index = index;
        anyIndex = (index == -2);
    };

    KeyClass(string key, bool dummy) {
        this->key = key;
        isStringKey = true;
        anyKey = (key.length() == 0 || key == "*");
    };
};

//------------------------------------------Stacks and Maps used for path query--------------------------------------------

vector <KeyClass> traversingPathKeysStack;
/*
  Currently, I am processing a value token that can be an immediate part of a list/object. this stack maintains this information
  When I got a list/object end token I pop this stack
  When I got a list/object start token I push list/object value in this stack
*/
vector <string> traversingInListOrObjectStack;
/*
  Example:
  $.*.info for this query $.bookstore.info and $.superstore.info need to be added in result
  for this case $.bookstore.info and $.superstore.info keys value will be stored in separate keys in this map
*/
map <string, string> jsonPathQueryResultsMap;
vector <string> jsonPathQueryResultKeys;
map <string, StreamToken> jsonPathQueryResultsLastAddedTokenMap;

//--------------------------------------------------------------------------------------------------------------------------
JsonStreamEvent<string> currentEvent;
// Processed tokenized version of json path query
vector <KeyClass> jsonPathQueryTokenized;
bool multiResultExist = false;

string getCurrentJsonPathKey() {
    string jsonPathKey = "";
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
    return traversingPathKeysStack.size()
           && !traversingPathKeysStack[traversingPathKeysStack.size() - 1].isStringKey;
}

bool isAppendingDelimeterNeededBefore(const StreamToken streamToken, string currentKey) {
    /*
     Example 1:
     current Token : KEY TOKEN
     current result Json : { 'name': 'Samin Yeaser'
     last added token: VALUE TOKEN
     So before adding current token to the JSON result I need to add a delimeter

     Example 2:
     current Token : KEY TOKEN
     current result Json : {
     last added token: VALUE TOKEN
     So before adding current token to the JSON result there is no need to add a delimeter
     This function determines if I need to add a delimeter or not
    */
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

void
addTokenInCurrentJsonPathResult(const StreamToken streamToken, string currentKey, bool currentKeyNotValid = false) {
    string resultInCurrentPath = jsonPathQueryResultsMap[currentKey];
    bool isNewKey = jsonPathQueryResultsLastAddedTokenMap[currentKey].isDefault;
    resultInCurrentPath.append(isAppendingDelimeterNeededBefore(streamToken, currentKey) ? "," : "");
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

bool isCurrentKeyMatchJsonPathQuery() {
    if (jsonPathQueryTokenized.size() > traversingPathKeysStack.size())
        return false;
    for (int i = 0; i < jsonPathQueryTokenized.size(); i++) {
        KeyClass currentKey = traversingPathKeysStack[i];
        KeyClass jsonPathQueryKey = jsonPathQueryTokenized[i];
        if ((jsonPathQueryKey.isStringKey != currentKey.isStringKey)
            || (jsonPathQueryKey.isStringKey && !(jsonPathQueryKey.anyKey || jsonPathQueryKey.key == currentKey.key))
            || (!jsonPathQueryKey.isStringKey
                && (!((jsonPathQueryKey.anyIndex && currentKey.index != -1) ||
                      jsonPathQueryKey.index == currentKey.index)))) {
            return false;
        }
    }
    return true;
}

void popCurrentTraversedKeysStack() {
    if (traversingPathKeysStack.size()) {
        traversingPathKeysStack.pop_back();
    }
}

void popCurrentlyTraversingInListOrObjectStack() {
    if (traversingInListOrObjectStack.size() > 0) {
        traversingInListOrObjectStack.pop_back();
    }
}

void IncrementIndexInTopKeyTraversedPathKeysStack() {
    if (isLastKeyOfCurrentPathIndex()) {
        traversingPathKeysStack[traversingPathKeysStack.size() - 1].index++;
    }
}

string getCurrentTokenIsPartOfObjectOrList() {
    return traversingInListOrObjectStack.size() == 0 ? "" :
           traversingInListOrObjectStack[traversingInListOrObjectStack.size() - 1];
}

void handleNewListStarted() {
    if (getCurrentTokenIsPartOfObjectOrList() == "list" && isLastKeyOfCurrentPathIndex())
        IncrementIndexInTopKeyTraversedPathKeysStack();
    traversingPathKeysStack.push_back(KeyClass(-1));
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

void setCurrentlyTraversingListOrObject(string value) {
    traversingInListOrObjectStack.push_back(value);
}

void handleJsonStreamParserEvent(const JsonStreamEvent<string> &jsonStreamEvent) {
    currentEvent = jsonStreamEvent;
    StreamToken streamToken = jsonStreamEvent.getStreamToken();
    bool ignoreEventFlag = false;
    bool shouldAddThisEvent = false;
    bool previousKeyValid = isCurrentKeyMatchJsonPathQuery();
    string previousKey = getCurrentJsonPathKey();
    string finalResult = "";

    if (streamToken.tokenType == JsonEventType::KEY_TOKEN) {
        ignoreEventFlag = ignoreEventFlag || !isCurrentKeyMatchJsonPathQuery();
        traversingPathKeysStack.push_back(KeyClass(streamToken.value, true));
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
    bool currentKeyMatched = isCurrentKeyMatchJsonPathQuery();
    if (currentKeyMatched && !ignoreEventFlag) {
        string currentKey = getCurrentJsonPathKey();
        addTokenInCurrentJsonPathResult(streamToken, currentKey + "");
    } else if (!currentKeyMatched && shouldAddThisEvent && previousKeyValid) {
        addTokenInCurrentJsonPathResult(streamToken, previousKey, !currentKeyMatched);
    }
    if (streamToken.tokenType == DOCUMENT_END_TOKEN) {
        for (auto it = jsonPathQueryResultKeys.begin(); it != jsonPathQueryResultKeys.end(); it++) {
            string jsonPath = *it;
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
        std::cout << "Got value of desired key final result: " << endl
                  << finalResult << endl;
        // if (finalResult.length()) {
        //   cout<<finalResult<<","<<endl;
        // }
    }
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
    string curr = "";
    bool listIndex = false;
    int lastListEnd = -1, lastDotIndex = -1;

    for (int i = 0; i < jsonPathQuery.size(); i++) {
        if (jsonPathQuery[i] == '.') {
            if (i - 1 != lastListEnd) {
                addKeyToJsonPathQueryProcessedList(listIndex, curr);
                lastDotIndex = i;
            }
            curr = "";
        } else if (jsonPathQuery[i] == '[') {
            if (i - 1 != lastDotIndex && i - 1 != lastListEnd) {
                addKeyToJsonPathQueryProcessedList(listIndex, curr);
            }
            listIndex = true;
            curr = "";
        } else if (jsonPathQuery[i] == ']') {
            addKeyToJsonPathQueryProcessedList(listIndex, curr);
            listIndex = false;
            curr = "";
            lastListEnd = i;
        } else
            curr.push_back(jsonPathQuery[i]);
    }
    if (curr.length())
        addKeyToJsonPathQueryProcessedList(listIndex, curr);

    for (int i = 0; i < jsonPathQueryTokenized.size(); i++) {
        multiResultExist = multiResultExist || (jsonPathQueryTokenized[i].anyKey || jsonPathQueryTokenized[i].anyIndex);
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