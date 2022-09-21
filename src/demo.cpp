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
// Currently I am reading token that can be immediate part of a list/object. this stack maintain this information
// When I got an list/object end token I pop stack
// When I got an list/object start token I push list/object value in the stack 
vector<string> currentlyTraversingInListOrObjectStack;
// Example:
// $.*.info for this query $.bookstore.info and $.superstore.info need to be added in result
// for this case $.bookstore.info and $.superstore.info keys value will be stored in separate keys in this map
map<string, string> jsonPathQueryResultsMap;
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

bool isLastKeyInKeyStackIsIndex() { return currentTraversedPathKeysStack.size() && !currentTraversedPathKeysStack[currentTraversedPathKeysStack.size() - 1].isStringKey; }

bool isAppendingDelimeterNeeded(const StreamToken streamToken, string currentKey) 
{
  StreamToken lastAddedStreamToken = jsonPathQueryResultsLastAddedTokenMap[currentKey];
  if (lastAddedStreamToken.isDefault) return false;
  if (streamToken.token_sub_type == LIST_ENDED || streamToken.token_sub_type == OBJECT_ENDED) {
    return false;
  }
  if (streamToken.tokenType==VALUE_EVENT || streamToken.token_sub_type == LIST_STARTED || streamToken.token_sub_type == OBJECT_STARTED) {
    return lastAddedStreamToken.tokenType==VALUE_EVENT || lastAddedStreamToken.token_sub_type==OBJECT_ENDED || lastAddedStreamToken.token_sub_type==LIST_ENDED;
  }
  if (streamToken.tokenType==KEY_EVENT) {
    return lastAddedStreamToken.token_sub_type!=OBJECT_STARTED;
  }
  return false;
  
}

void addTokenInCurrentJsonPathResult(const StreamToken streamToken, string currentKey, bool listEndTagCheck = false)
{  
  string oneOfTheDesiredJsonFromSpecificPath = jsonPathQueryResultsMap[currentKey];
  oneOfTheDesiredJsonFromSpecificPath.append(isAppendingDelimeterNeeded(streamToken, currentKey)? ",":"");
  jsonPathQueryResultsLastAddedTokenMap[currentKey]=streamToken;

  if (streamToken.tokenType == JsonEventType::KEY_EVENT)
  {
    oneOfTheDesiredJsonFromSpecificPath += "\"" + streamToken.value + "\"" + " : ";
  }
  if (streamToken.tokenType == VALUE_EVENT)
  {
    oneOfTheDesiredJsonFromSpecificPath.append(streamToken.token_sub_type == JsonEventType::STRING_EVENT? "\"" + streamToken.value + "\"": streamToken.value);
  }
  if (streamToken.token_sub_type == LIST_STARTED)
  {
    oneOfTheDesiredJsonFromSpecificPath.push_back('[');
  }
  if (streamToken.token_sub_type == LIST_ENDED && (!listEndTagCheck || (currentTraversedPathKeysStack.size() + 1 == jsonPathQueryTokenized.size() && jsonPathQueryTokenized[jsonPathQueryTokenized.size() - 1].isStringKey)))
  {
    oneOfTheDesiredJsonFromSpecificPath.push_back(']');
  }
  if (streamToken.token_sub_type == OBJECT_STARTED)
  {
    oneOfTheDesiredJsonFromSpecificPath.push_back('{');
  }
  if (streamToken.token_sub_type == OBJECT_ENDED)
  {
    oneOfTheDesiredJsonFromSpecificPath.push_back('}');
  }
  jsonPathQueryResultsMap[currentKey] = oneOfTheDesiredJsonFromSpecificPath;
}

bool isCurrentKeySatisfyJsonPathQuery()
{
  if (jsonPathQueryTokenized.size() > currentTraversedPathKeysStack.size()) return false; 
  for (int i = 0; i < jsonPathQueryTokenized.size(); i++)
  {
    KeyClass currentKey = currentTraversedPathKeysStack[i], jsonPathQueryKey = jsonPathQueryTokenized[i];
    if ((jsonPathQueryKey.isStringKey != currentKey.isStringKey) || (jsonPathQueryKey.isStringKey && !(jsonPathQueryKey.anyKey || jsonPathQueryKey.key == currentKey.key)) || (!jsonPathQueryKey.isStringKey && (!((jsonPathQueryKey.anyIndex && currentKey.index != -1) || jsonPathQueryKey.index == currentKey.index))))
    {
      return false;;
    }
  }
  return true;
}

void popKeyFromTraversedKeysStack()
{
  if (currentTraversedPathKeysStack.size()) currentTraversedPathKeysStack.pop_back();
}

void popCurrentlyTraversingInListOrObjectStack()
{
  if (currentlyTraversingInListOrObjectStack.size() > 0) currentlyTraversingInListOrObjectStack.pop_back();
}

void IncrementIndexInTraversedPathKeysStack()
{
  if (isLastKeyInKeyStackIsIndex()) currentTraversedPathKeysStack[currentTraversedPathKeysStack.size() - 1].index++;
}

string getCurrentTokenIsPartOfObjectOrList()
{
  return currentlyTraversingInListOrObjectStack.size() == 0 ? "" : currentlyTraversingInListOrObjectStack[currentlyTraversingInListOrObjectStack.size() - 1];
}

void handleNewListStarted()
{
  if (getCurrentTokenIsPartOfObjectOrList() == "list" && isLastKeyInKeyStackIsIndex()) IncrementIndexInTraversedPathKeysStack();
  currentTraversedPathKeysStack.push_back(KeyClass(-1));
}
void handleListEnded()
{
  popCurrentlyTraversingInListOrObjectStack();
  if (isLastKeyInKeyStackIsIndex())  popKeyFromTraversedKeysStack();
  if (getCurrentTokenIsPartOfObjectOrList() == "object") popKeyFromTraversedKeysStack();
}


void handleObjectEnded()
{
  popCurrentlyTraversingInListOrObjectStack();
  if (getCurrentTokenIsPartOfObjectOrList() == "object") popKeyFromTraversedKeysStack();
}

void handleNewValueAddedInList()
{
  if (getCurrentTokenIsPartOfObjectOrList() == "list" && isLastKeyInKeyStackIsIndex()) IncrementIndexInTraversedPathKeysStack();
}

void setCurrentlyTraversingListOrObject(string value) { currentlyTraversingInListOrObjectStack.push_back(value); }

void handleJsonStreamParserEvents(const JsonStreamEvent<string> &jsonStreamEvent)
{
  currentEvent = jsonStreamEvent;
  StreamToken streamToken = jsonStreamEvent.getStreamToken();
  bool ignoreEventFlag = false;
  bool shouldAddThisEvent = false;
  bool currentlyValid = isCurrentKeySatisfyJsonPathQuery();
  bool should_check_list_end_symbol_append = false;
  string previousKey = getCurrentJsonPathKey();
  string finalResult = "";

  if (streamToken.tokenType == JsonEventType::KEY_EVENT)
  {
    ignoreEventFlag = ignoreEventFlag || !isCurrentKeySatisfyJsonPathQuery();
    currentTraversedPathKeysStack.push_back(KeyClass(streamToken.value, true));
  }
  else if (streamToken.tokenType == VALUE_EVENT)
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
  else if (streamToken.token_sub_type == LIST_STARTED)
  {
    handleNewListStarted();
    setCurrentlyTraversingListOrObject("list");
  }
  else if (streamToken.token_sub_type == LIST_ENDED)
  {
    handleListEnded();
    shouldAddThisEvent = should_check_list_end_symbol_append = true;
  }
  else if (streamToken.token_sub_type == OBJECT_STARTED)
  {
    if (getCurrentTokenIsPartOfObjectOrList() == "list") handleNewValueAddedInList();
    setCurrentlyTraversingListOrObject("object");
  }
  else if (streamToken.token_sub_type == OBJECT_ENDED)
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
    addTokenInCurrentJsonPathResult(streamToken, previousKey, should_check_list_end_symbol_append);
  }
  if (streamToken.tokenType == Document_END)
  {
    for (auto it = jsonPathQueryResultsMap.begin(); it != jsonPathQueryResultsMap.end(); it++)
    {
      string key = it->first;
      string value = it->second;
      // cout<<"All answers: "<<key<<" -> "<<value<<endl;
      if (finalResult.length()) finalResult.push_back(',');
      finalResult.append(value);
    }
    if (multiResultExist) { finalResult = "[" + finalResult + "]"; }
    std::cout << "Got value of desired key final result: " << endl << finalResult << endl;
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
      if (i - 1 != lastDotIndex && i - 1 != lastListEnd) addKeyToJsonPathQueryProcessedList(listIndex, curr);
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
    else curr.push_back(jsonPathQuery[i]);
  }
  if (curr.length()) addKeyToJsonPathQueryProcessedList(listIndex, curr);

  for (int i = 0; i < jsonPathQueryTokenized.size(); i++)
  {
    multiResultExist = multiResultExist || (jsonPathQueryTokenized[i].anyKey || jsonPathQueryTokenized[i].anyIndex);
  }
}

int main(int argc, char **argv)
{
  freopen("output.txt","a+",stdout);
  JsonStreamParser jsonStreamParser = JsonStreamParser();
  string fileName(argv[1]);
  string jsonPathQuery(argv[2]);
  cout<<"JSON path query:"<<jsonPathQuery<<" -filename: "<<fileName<<endl;
  fileName = "tests/Json files/" + fileName;
  KeyClass keyClass("$", true);
  currentTraversedPathKeysStack.push_back(keyClass);
  processJsonPathQuery(jsonPathQuery);
  jsonStreamParser.setEventHandler(handleJsonStreamParserEvents);
  jsonStreamParser.startJsonStreaming(fileName);
}