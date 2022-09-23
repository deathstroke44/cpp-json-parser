#include "library/code.hh"
using namespace std;
class KeyClass
{
public:
  bool isStringKey = false, anyIndex = false, anyKey = false;
  int index;
  string key;
  KeyClass() = default;
  KeyClass(int index)
  {
    this->index = index;
    anyIndex = (index == -2);
  };
  KeyClass(string key, bool dummy)
  {
    this->key = key;
    isStringKey = true;
    anyKey = (key.length() == 0 || key == "*");
  };
};

//------------------------------------------Stacks and Maps used for path query--------------------------------------------

vector<KeyClass> currentTraversedPathKeysStack;
// Currently, I am processing a value token that can be an immediate part of a list/object. this stack maintains this information
// When I got a list/object end token I pop this stack
// When I got a list/object start token I push list/object value in this stack
vector<string> currentlyTraversingInListOrObjectStack;
// Example:
// $.*.info for this query $.bookstore.info and $.superstore.info need to be added in result
// for this case $.bookstore.info and $.superstore.info keys value will be stored in separate keys in this map
map<string, string> jsonPathQueryResultsMap;
vector<string> jsonPathQueryResultKeys;
map<string, StreamToken> jsonPathQueryResultsLastAddedTokenMap;

//--------------------------------------------------------------------------------------------------------------------------
JsonStreamEvent<string> currentEvent;
// Processed tokenized version of json path query
vector<KeyClass> jsonPathQueryTokenized;
bool multiResultExist = false;

string getCurrentJsonPathKey()
{
  string str = "";
  for (int i = 0; i < jsonPathQueryTokenized.size() && i < currentTraversedPathKeysStack.size(); i++)
  {
    KeyClass kc = currentTraversedPathKeysStack[i];
    str = !kc.isStringKey ? str + "[" + to_string(kc.index) + "]" : str = str + "." + kc.key;
  }
  return str;
}

bool isLastKeyOfCurrentPathIsIndex()
{
  return currentTraversedPathKeysStack.size() && !currentTraversedPathKeysStack[currentTraversedPathKeysStack.size() - 1].isStringKey;
}

bool isAppendingDelimeterNeeded(const StreamToken streamToken, string currentKey)
{
  StreamToken lastAddedStreamToken = jsonPathQueryResultsLastAddedTokenMap[currentKey];
  if (lastAddedStreamToken.isDefault)
    return false;
  if (streamToken.tokenType == LIST_ENDED_TOKEN || streamToken.tokenType == OBJECT_ENDED_TOKEN)
  {
    return false;
  }
  if (streamToken.tokenType == VALUE_TOKEN || streamToken.tokenType == LIST_STARTED_TOKEN || streamToken.tokenType == OBJECT_STARTED_TOKEN)
  {
    return lastAddedStreamToken.tokenType == VALUE_TOKEN || lastAddedStreamToken.tokenType == OBJECT_ENDED_TOKEN || lastAddedStreamToken.tokenType == LIST_ENDED_TOKEN;
  }
  if (streamToken.tokenType == KEY_TOKEN)
  {
    return lastAddedStreamToken.tokenType != OBJECT_STARTED_TOKEN;
  }
  return false;
}

void addTokenInCurrentJsonPathResult(const StreamToken streamToken, string currentKey, bool listEndTagCheck = false)
{
  string oneOfTheDesiredJsonFromSpecificPath = jsonPathQueryResultsMap[currentKey];
  bool isThisKeyNewKey = jsonPathQueryResultsLastAddedTokenMap[currentKey].isDefault;
  oneOfTheDesiredJsonFromSpecificPath.append(isAppendingDelimeterNeeded(streamToken, currentKey) ? "," : "");
  jsonPathQueryResultsLastAddedTokenMap[currentKey] = streamToken;
  bool tokenAdded = true;

  if (streamToken.tokenType == JsonEventType::KEY_TOKEN)
  {
    oneOfTheDesiredJsonFromSpecificPath += "\"" + streamToken.value + "\"" + " : ";
  }
  if (streamToken.tokenType == VALUE_TOKEN)
  {
    oneOfTheDesiredJsonFromSpecificPath.append(streamToken.isStringValue ? "\"" + streamToken.value + "\"" : streamToken.value);
  }
  if (streamToken.tokenType == LIST_STARTED_TOKEN)
  {
    oneOfTheDesiredJsonFromSpecificPath.push_back('[');
  }
  if (streamToken.tokenType == LIST_ENDED_TOKEN && (!listEndTagCheck || (currentTraversedPathKeysStack.size() + 1 == jsonPathQueryTokenized.size() && jsonPathQueryTokenized[jsonPathQueryTokenized.size() - 1].isStringKey)))
  {
    oneOfTheDesiredJsonFromSpecificPath.push_back(']');
  }
  else if (streamToken.tokenType == LIST_ENDED_TOKEN)
  {
    tokenAdded = false;
  }
  if (streamToken.tokenType == OBJECT_STARTED_TOKEN)
  {
    oneOfTheDesiredJsonFromSpecificPath.push_back('{');
  }
  if (streamToken.tokenType == OBJECT_ENDED_TOKEN)
  {
    oneOfTheDesiredJsonFromSpecificPath.push_back('}');
  }
  if (tokenAdded)
  {
    if (isThisKeyNewKey)
    {
      jsonPathQueryResultKeys.push_back(currentKey);
    }
    jsonPathQueryResultsMap[currentKey] = oneOfTheDesiredJsonFromSpecificPath;
  }
}

bool isCurrentKeySatisfyJsonPathQuery()
{
  if (jsonPathQueryTokenized.size() > currentTraversedPathKeysStack.size())
    return false;
  for (int i = 0; i < jsonPathQueryTokenized.size(); i++)
  {
    KeyClass currentKey = currentTraversedPathKeysStack[i], jsonPathQueryKey = jsonPathQueryTokenized[i];
    if ((jsonPathQueryKey.isStringKey != currentKey.isStringKey) || (jsonPathQueryKey.isStringKey && !(jsonPathQueryKey.anyKey || jsonPathQueryKey.key == currentKey.key)) || (!jsonPathQueryKey.isStringKey && (!((jsonPathQueryKey.anyIndex && currentKey.index != -1) || jsonPathQueryKey.index == currentKey.index))))
    {
      return false;
      ;
    }
  }
  return true;
}

void popKeyFromTraversedKeysStack()
{
  if (currentTraversedPathKeysStack.size())
    currentTraversedPathKeysStack.pop_back();
}

void popCurrentlyTraversingInListOrObjectStack()
{
  if (currentlyTraversingInListOrObjectStack.size() > 0)
    currentlyTraversingInListOrObjectStack.pop_back();
}

void IncrementIndexInTraversedPathKeysStack()
{
  if (isLastKeyOfCurrentPathIsIndex())
    currentTraversedPathKeysStack[currentTraversedPathKeysStack.size() - 1].index++;
}

string getCurrentTokenIsPartOfObjectOrList()
{
  return currentlyTraversingInListOrObjectStack.size() == 0 ? "" : currentlyTraversingInListOrObjectStack[currentlyTraversingInListOrObjectStack.size() - 1];
}

void handleNewListStarted()
{
  if (getCurrentTokenIsPartOfObjectOrList() == "list" && isLastKeyOfCurrentPathIsIndex())
    IncrementIndexInTraversedPathKeysStack();
  currentTraversedPathKeysStack.push_back(KeyClass(-1));
}
void handleListEnded()
{
  popCurrentlyTraversingInListOrObjectStack();
  if (isLastKeyOfCurrentPathIsIndex())
    popKeyFromTraversedKeysStack();
  if (getCurrentTokenIsPartOfObjectOrList() == "object")
    popKeyFromTraversedKeysStack();
}

void handleObjectEnded()
{
  popCurrentlyTraversingInListOrObjectStack();
  if (getCurrentTokenIsPartOfObjectOrList() == "object")
    popKeyFromTraversedKeysStack();
}

void handleNewValueAddedInList()
{
  if (getCurrentTokenIsPartOfObjectOrList() == "list" && isLastKeyOfCurrentPathIsIndex())
    IncrementIndexInTraversedPathKeysStack();
}

void setCurrentlyTraversingListOrObject(string value)
{
  currentlyTraversingInListOrObjectStack.push_back(value);
}

void handleJsonStreamParserEvent(const JsonStreamEvent<string> &jsonStreamEvent)
{
  currentEvent = jsonStreamEvent;
  StreamToken streamToken = jsonStreamEvent.getStreamToken();
  bool ignoreEventFlag = false;
  bool shouldAddThisEvent = false;
  bool currentlyValid = isCurrentKeySatisfyJsonPathQuery();
  bool shouldCheckListEndSymbolAppend = false;
  string previousKey = getCurrentJsonPathKey();
  string finalResult = "";

  if (streamToken.tokenType == JsonEventType::KEY_TOKEN)
  {
    ignoreEventFlag = ignoreEventFlag || !isCurrentKeySatisfyJsonPathQuery();
    currentTraversedPathKeysStack.push_back(KeyClass(streamToken.value, true));
  }
  else if (streamToken.tokenType == VALUE_TOKEN)
  {
    if (getCurrentTokenIsPartOfObjectOrList() == "object")
    {
      popKeyFromTraversedKeysStack();
      shouldAddThisEvent = true;
    }
    else if (getCurrentTokenIsPartOfObjectOrList() == "list")
    {
      handleNewValueAddedInList();
    }
  }
  else if (streamToken.tokenType == LIST_STARTED_TOKEN)
  {
    handleNewListStarted();
    setCurrentlyTraversingListOrObject("list");
  }
  else if (streamToken.tokenType == LIST_ENDED_TOKEN)
  {
    handleListEnded();
    shouldAddThisEvent = shouldCheckListEndSymbolAppend = true;
  }
  else if (streamToken.tokenType == OBJECT_STARTED_TOKEN)
  {
    if (getCurrentTokenIsPartOfObjectOrList() == "list")
      handleNewValueAddedInList();
    setCurrentlyTraversingListOrObject("object");
  }
  else if (streamToken.tokenType == OBJECT_ENDED_TOKEN)
  {
    handleObjectEnded();
    shouldAddThisEvent = true;
  }
  bool fg = isCurrentKeySatisfyJsonPathQuery();
  if (fg && !ignoreEventFlag)
  {
    string currentKey = getCurrentJsonPathKey();
    addTokenInCurrentJsonPathResult(streamToken, currentKey + "");
  }
  else if (!fg && shouldAddThisEvent && currentlyValid)
  {
    addTokenInCurrentJsonPathResult(streamToken, previousKey, shouldCheckListEndSymbolAppend);
  }
  if (streamToken.tokenType == DOCUMENT_END_TOKEN)
  {
    for (auto it = jsonPathQueryResultKeys.begin(); it != jsonPathQueryResultKeys.end(); it++)
    {
      string key = *it;
      string value = jsonPathQueryResultsMap[key];
      cout << "All answers: " << key << " -> " << value << endl;
      if (finalResult.length())
        finalResult.push_back(',');
      finalResult.append(value);
    }
    if (multiResultExist)
    {
      finalResult = "[" + finalResult + "]";
    }
    std::cout << "Got value of desired key final result: " << endl
              << finalResult << endl;
    // if (finalResult.length()) {
    //   cout<<finalResult<<","<<endl;
    // }
  }
}

void addKeyToJsonPathQueryProcessedList(bool listIndex, string val)
{
  if (listIndex)
  {
    KeyClass keyClass(val == "*" ? -2 : stoi(val));
    jsonPathQueryTokenized.push_back(keyClass);
  }
  else
  {
    KeyClass keyClass(val, true);
    jsonPathQueryTokenized.push_back(keyClass);
  }
}
void processJsonPathQuery(string jsonPathQuery)
{
  string curr = "";
  bool listIndex = false;
  int lastListEnd = -1, lastDotIndex = -1;

  for (int i = 0; i < jsonPathQuery.size(); i++)
  {
    if (jsonPathQuery[i] == '.')
    {
      if (i - 1 != lastListEnd)
      {
        addKeyToJsonPathQueryProcessedList(listIndex, curr);
        lastDotIndex = i;
      }
      curr = "";
    }
    else if (jsonPathQuery[i] == '[')
    {
      if (i - 1 != lastDotIndex && i - 1 != lastListEnd)
        addKeyToJsonPathQueryProcessedList(listIndex, curr);
      listIndex = true;
      curr = "";
    }
    else if (jsonPathQuery[i] == ']')
    {
      addKeyToJsonPathQueryProcessedList(listIndex, curr);
      listIndex = false;
      curr = "";
      lastListEnd = i;
    }
    else
      curr.push_back(jsonPathQuery[i]);
  }
  if (curr.length())
    addKeyToJsonPathQueryProcessedList(listIndex, curr);

  for (int i = 0; i < jsonPathQueryTokenized.size(); i++)
  {
    multiResultExist = multiResultExist || (jsonPathQueryTokenized[i].anyKey || jsonPathQueryTokenized[i].anyIndex);
  }
}

int main(int argc, char **argv)
{
  freopen("output.txt", "a+", stdout);
  JsonStreamParser jsonStreamParser = JsonStreamParser();
  string fileName(argv[1]);
  string jsonPathQuery(argv[2]);
  cout << "JSON path query:" << jsonPathQuery << " -filename: " << fileName << endl;
  fileName = "tests/Json files/" + fileName;
  KeyClass keyClass("$", true);
  currentTraversedPathKeysStack.push_back(keyClass);
  processJsonPathQuery(jsonPathQuery);
  jsonStreamParser.setEventHandler(handleJsonStreamParserEvent);
  jsonStreamParser.startJsonStreaming(fileName);
}