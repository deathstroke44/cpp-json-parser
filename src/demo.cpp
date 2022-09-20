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
vector<KeyClass> CurrentPathKeys;
// Currently I am reading token that can be immediate part of a list/object. this stack maintain this information
// When I got an list/object end token I pop stack
// When I got an list/object start token I push list/object value in the stack 

vector<string> currentlyInListOrObjectStack;
JsonStreamEvent<string> current_event;
// Example:
// $.*.info for this query $.bookstore.info and $.superstore.info need to be added in result
// for this case $.bookstore.info and $.superstore.info keys value will be stored in separate keys in this map
map<string, string> jsonPathQueryResultsMap;
map<string, StreamToken> jsonPathQueryResultLastAddedToken;
// Processed tokenized version of json path query
vector<KeyClass> jsonPathQueryTokenized;
bool multiResultExist = false;
string getCurrentJsonPathQueryKey()
{
  string str = "";
  for (int i = 0; i < jsonPathQueryTokenized.size() && i < CurrentPathKeys.size(); i++)
  {
    KeyClass kc = CurrentPathKeys[i];
    str = (!kc.isStringKey) ? str + "." + "[" + to_string(kc.index) + "]" : str = str + "." + kc.key;
  }
  return str;
}
string getKeyValue(string key) { return jsonPathQueryResultsMap[key]; }
bool isLastKeyInKeyStackIsIndex() { return CurrentPathKeys.size() && !CurrentPathKeys[CurrentPathKeys.size() - 1].isStringKey; }
void putKeyValue(string key, string value) { jsonPathQueryResultsMap[key] = value; }
bool addingDelimeterNeeded(const StreamToken streamToken, string currentKey) {
  StreamToken lastAddedStreamToken = jsonPathQueryResultLastAddedToken[currentKey];
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
void addTokenInResult(const StreamToken streamToken, string currentKey, bool listEndTagCheck = false)
{  
  bool shouldAppendDelimeter = addingDelimeterNeeded(streamToken, currentKey);
  string oneOfTheDesiredJsonFromSpecificPath = getKeyValue(currentKey);
  if (shouldAppendDelimeter) {
    oneOfTheDesiredJsonFromSpecificPath.push_back(',');
  }
  jsonPathQueryResultLastAddedToken[currentKey]=streamToken;

  if (streamToken.tokenType == JsonEventType::KEY_EVENT)
  {
    oneOfTheDesiredJsonFromSpecificPath += "\"" + streamToken.value + "\"" + " : ";
  }
  if (streamToken.tokenType == VALUE_EVENT)
  {
    if (streamToken.token_sub_type == JsonEventType::STRING_EVENT)
    {
      oneOfTheDesiredJsonFromSpecificPath += "\"" + streamToken.value + "\"";
    }
    else
    {
      oneOfTheDesiredJsonFromSpecificPath += streamToken.value;
    }
  }
  if (streamToken.tokenType == JsonEventType::OBJECT_LIST_EVENT)
  {
    if (streamToken.value == "list started")
    {
      oneOfTheDesiredJsonFromSpecificPath.push_back('[');
    }
    else if (streamToken.value == "list ended" && (!listEndTagCheck || (CurrentPathKeys.size() + 1 == jsonPathQueryTokenized.size() && jsonPathQueryTokenized[jsonPathQueryTokenized.size() - 1].isStringKey)))
    {
      oneOfTheDesiredJsonFromSpecificPath.push_back(']');
    }
    else if (streamToken.value == "object started")
    {
      oneOfTheDesiredJsonFromSpecificPath.push_back('{');
    }
    else if (streamToken.value == "object ended")
    {
      oneOfTheDesiredJsonFromSpecificPath.push_back('}');
    }
  }
  putKeyValue(currentKey, oneOfTheDesiredJsonFromSpecificPath);
}
bool currentKeyPartOfDesiredKey()
{
  if (jsonPathQueryTokenized.size() <= CurrentPathKeys.size())
  {
    for (int i = 0; i < jsonPathQueryTokenized.size(); i++)
    {
      KeyClass kcC = CurrentPathKeys[i];
      KeyClass kcD = jsonPathQueryTokenized[i];
      if (kcD.isStringKey != kcC.isStringKey)
        return false;
      if (kcD.isStringKey)
      {
        if (!(kcD.anyKey || kcD.key == kcC.key))
        {
          return false;
        }
      }
      else
      {
        if (!((kcD.anyIndex && kcC.index != -1) || kcD.index == kcC.index))
        {
          return false;
        }
      }
    }
  }
  else
  {
    return false;
  }
  return true;
}

void popKey()
{
  if (CurrentPathKeys.size())
    CurrentPathKeys.pop_back();
}
void IncrementIndexKey()
{
  if (isLastKeyInKeyStackIsIndex())
    CurrentPathKeys[CurrentPathKeys.size() - 1].index++;
}

void pushStringKey(string key)
{
  CurrentPathKeys.push_back(KeyClass(key, true));
}

void pushIndexKey(int index)
{
  CurrentPathKeys.push_back(KeyClass(index));
}

string getPartOfValue()
{
  return currentlyInListOrObjectStack.size() == 0 ? "" : currentlyInListOrObjectStack[currentlyInListOrObjectStack.size() - 1];
}

void setNewListStarted()
{
  if (getPartOfValue() == "list" && isLastKeyInKeyStackIsIndex())
  {
    IncrementIndexKey();
  }
  pushIndexKey(-1);
}
void popPartOfValue()
{
  if (currentlyInListOrObjectStack.size() > 0)
  {
    currentlyInListOrObjectStack.pop_back();
  }
}

void setLastObjectEnded()
{
  popPartOfValue();
  if (getPartOfValue() == "object")
  {
    popKey();
  }
}

void setNewValueAddedInList()
{
  if (getPartOfValue() == "list" && isLastKeyInKeyStackIsIndex())
  {
    IncrementIndexKey();
  }
}

void setPartOfValue(string value) { currentlyInListOrObjectStack.push_back(value); }

void handleEvent(const JsonStreamEvent<string> &event)
{
  current_event = event;
  StreamToken streamToken = event.getStreamToken();
  bool ignoreEventFlag = false;
  bool shouldAddThisEvent = false;
  bool currentlyValid = currentKeyPartOfDesiredKey();
  bool should_check_list_end_symbol_append = false;
  string previousKey = getCurrentJsonPathQueryKey();
  string finalResult = "";

  if (streamToken.tokenType == JsonEventType::KEY_EVENT)
  {
    ignoreEventFlag = ignoreEventFlag || !currentKeyPartOfDesiredKey();
    pushStringKey(streamToken.value);
  }
  else if (streamToken.tokenType == VALUE_EVENT)
  {
    if (getPartOfValue() == "object")
    {
      popKey();
      shouldAddThisEvent = true;
    }
    else if (getPartOfValue() == "list")
    {
      setNewValueAddedInList();
    }
  }
  else if (streamToken.token_sub_type == LIST_STARTED)
  {
    setNewListStarted();
    setPartOfValue("list");
  }
  else if (streamToken.token_sub_type == LIST_ENDED)
  {
    popPartOfValue();
    if (isLastKeyInKeyStackIsIndex())
    {
      popKey();
    }
    if (getPartOfValue() == "object")
    {
      popKey();
    }
    shouldAddThisEvent = true;
    should_check_list_end_symbol_append = true;
  }
  else if (streamToken.token_sub_type == OBJECT_STARTED)
  {
    if (getPartOfValue() == "list")
    {
      setNewValueAddedInList();
    }
    setPartOfValue("object");
  }
  else if (streamToken.token_sub_type == OBJECT_ENDED)
  {
    setLastObjectEnded();
    shouldAddThisEvent = true;
  }
  
  bool fg = currentKeyPartOfDesiredKey();
  if (fg && !ignoreEventFlag)
  {
    string currentKey = getCurrentJsonPathQueryKey();
    addTokenInResult(streamToken, currentKey + "");
  }
  else if (!fg && shouldAddThisEvent && currentlyValid)
  {
    addTokenInResult(streamToken, previousKey, should_check_list_end_symbol_append);
  }
  if (streamToken.tokenType == Document_END)
  {
    for (auto it = jsonPathQueryResultsMap.begin(); it != jsonPathQueryResultsMap.end(); it++)
    {
      string key = it->first;
      string value = it->second;
      if (finalResult.length()) finalResult.push_back(',');
      finalResult.append(value);
    }
    if (multiResultExist)
    {
      finalResult = "[" + finalResult + "]";
    }
    std::cout << "Got value of desired key final result: " << endl
              << finalResult << endl;
  }
}

void addKey(bool listIndex, string val)
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
        addKey(listIndex, curr);
        lastDotIndex = i;
      }
      curr = "";
    }
    else if (jsonPathQuery[i] == '[')
    {
      if (i - 1 != lastDotIndex && i - 1 != lastListEnd)
        addKey(listIndex, curr);
      listIndex = true;
      curr = "";
    }
    else if (jsonPathQuery[i] == ']')
    {
      addKey(listIndex, curr);
      listIndex = false;
      curr = "";
      lastListEnd = i;
    }
    else
    {
      curr.push_back(jsonPathQuery[i]);
    }
  }
  if (curr.length())
    addKey(listIndex, curr);

  for (int i = 0; i < jsonPathQueryTokenized.size(); i++)
  {
    multiResultExist = multiResultExist || (jsonPathQueryTokenized[i].anyKey || jsonPathQueryTokenized[i].anyIndex);
  }
}

int main(int argc, char **argv)
{
  JsonStreamParser jsonStreamParser = JsonStreamParser();
  string fileName(argv[1]);
  string jsonPathQuery(argv[2]);
  fileName = "tests/Json files/" + fileName;
  KeyClass keyClass("$", true);
  CurrentPathKeys.push_back(keyClass);
  processJsonPathQuery(jsonPathQuery);
  jsonStreamParser.setEventHandler(handleEvent);
  jsonStreamParser.startJsonStreaming(fileName);
}