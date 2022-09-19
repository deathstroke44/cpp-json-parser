// code beautified using https://codebeautify.org/cpp-formatter-beautifier and https://formatter.org/cpp-formatter
#include "library/code.hh"
using namespace std;
class KeyClass {
 public:
  bool is_string_key = false;
  int index;
  bool any_index = false;
  bool any_key = false;
  string key;
  KeyClass() = default;
  KeyClass(int index) {
    this->index = index;
    any_index = (index == -2);
  };
  KeyClass(string key, bool dummy) {
    this->key = key;
    is_string_key = true;
    this->index = 0;
    any_key = (key.length() == 0 || key=="*");
  };
};
vector<KeyClass> key_stack;
vector<string> currentlyInListOrObjectStack;
JsonStreamEvent<string> current_event;
map<string, string> jsonPathQueryResultsMap;
Json_stream_parser json_stream_parser;
vector<KeyClass> jsonPathQueryTokenized;
bool multiResultExist = false;
string getCurrentJsonPathQueryKey() {
  string str = "";
  for (int i = 0; i < jsonPathQueryTokenized.size() && i < key_stack.size(); i++) {
    KeyClass kc = key_stack[i];
    if (!kc.is_string_key) {
      str = str + "." + "[" + to_string(kc.index) + "]";
    } else {
      str = str + "." + kc.key;
    }
  }
  return str;
}
string getKeyValue(string key) { return jsonPathQueryResultsMap[key];}
bool isLastKeyInKeyStackIsIndex() { return key_stack.size() && !key_stack[key_stack.size()-1].is_string_key;}
void putKeyValue(string key, string value) { jsonPathQueryResultsMap[key] = value; }
void removeDelimeterIfNeeded(const StreamToken streamToken, string currentKey) {
  string traversedJson = getKeyValue(currentKey);
  bool flag = streamToken.token_type == JsonEventType::OBJECT_LIST_EVENT &&
              ((streamToken.value == "list ended") ||
               (streamToken.value == "object ended"));
  if (flag && traversedJson.size() > 0 && traversedJson[traversedJson.size() - 1] == ',') {
    traversedJson.pop_back();
  }
  putKeyValue(currentKey, traversedJson);
}
string removeLastDelimeterIfNeeded(string key) {
  string traversedJson = getKeyValue(key);
  if (traversedJson.size() > 0 && traversedJson[traversedJson.size() - 1] == ',') {
    traversedJson.pop_back();
    putKeyValue(key, traversedJson);
  }
  return traversedJson;
}
void addTokenInDesiredResult(const StreamToken streamToken, string currentKey, bool listEndTagCheck=false) {
  removeDelimeterIfNeeded(streamToken, currentKey);
  string oneOfTheDesiredJsonFromSpecificPath = getKeyValue(currentKey);
  if (streamToken.token_type == JsonEventType::KEY_EVENT) {
    oneOfTheDesiredJsonFromSpecificPath += "\"" + streamToken.value + "\"" + " : ";
  }
  if (streamToken.token_type == VALUE_EVENT) {
    if (streamToken.token_sub_type == JsonEventType::STRING_EVENT) {
      oneOfTheDesiredJsonFromSpecificPath += "\"" + streamToken.value + "\",";
    } else {
      oneOfTheDesiredJsonFromSpecificPath += streamToken.value + ",";
    }
  }
  if (streamToken.token_type == JsonEventType::OBJECT_LIST_EVENT) {
    if (streamToken.value == "list started") {
      oneOfTheDesiredJsonFromSpecificPath.push_back('[');
    } else if (streamToken.value == "list ended") {
      if (!listEndTagCheck || (key_stack.size()+1==jsonPathQueryTokenized.size() && jsonPathQueryTokenized[jsonPathQueryTokenized.size()-1].is_string_key)) {
        oneOfTheDesiredJsonFromSpecificPath.push_back(']');
        oneOfTheDesiredJsonFromSpecificPath.push_back(',');
      }
    } else if (streamToken.value == "object started") {
      oneOfTheDesiredJsonFromSpecificPath.push_back('{');
    } else if (streamToken.value == "object ended") {
      oneOfTheDesiredJsonFromSpecificPath.push_back('}');
      oneOfTheDesiredJsonFromSpecificPath.push_back(',');
    }
  }
  putKeyValue(currentKey, oneOfTheDesiredJsonFromSpecificPath);
}
bool currentKeyPartOfDesiredKey() {
  if (jsonPathQueryTokenized.size() <= key_stack.size()) {
    for (int i = 0; i < jsonPathQueryTokenized.size(); i++) {
      KeyClass kcC = key_stack[i];
      KeyClass kcD = jsonPathQueryTokenized[i];
      if (kcD.is_string_key != kcC.is_string_key) return false;
      if (kcD.is_string_key) {
        if (!(kcD.any_key || kcD.key == kcC.key)) { return false; }
      } else {
        if (!((kcD.any_index && kcC.index != -1) || kcD.index == kcC.index)) { return false; }
      }
    }
  } 
  else {return false;}
  return true;
}

void pop_key() {
  if (key_stack.size()) {
    key_stack.pop_back();
  }
}
void IncrementIndexKey() {
  if (isLastKeyInKeyStackIsIndex()) key_stack[key_stack.size() - 1].index++;
}

void push_string_key(string key) {
  key_stack.push_back(KeyClass(key, true));
}

void pushIndexKey(int index) {
  key_stack.push_back(KeyClass(index));
}

string getPartOfValue() {
  if (currentlyInListOrObjectStack.size() == 0) return "";
  return currentlyInListOrObjectStack[currentlyInListOrObjectStack.size() - 1];
}

void setNewListStarted() {
  if (getPartOfValue() == "list" && isLastKeyInKeyStackIsIndex()) {
    IncrementIndexKey();
  }
  pushIndexKey(-1);
}
void pop_part_of_value() {
  if (currentlyInListOrObjectStack.size() > 0) {
    currentlyInListOrObjectStack.pop_back();
  }
}

void set_last_object_ended() {
  pop_part_of_value();
  if (getPartOfValue() == "object") {
    pop_key();
  }
}

void set_new_value_added_in_list() {
  if (getPartOfValue() == "list" && isLastKeyInKeyStackIsIndex()) {
    IncrementIndexKey();
  }
}

void setPartOfValue(string value) { currentlyInListOrObjectStack.push_back(value); }

void handleEvent(const JsonStreamEvent<string>& event) {
  current_event = event;
  StreamToken streamToken = event.getStreamToken();
  bool ignoreEventFlag = false;
  bool shouldAddThisEvent = false;
  bool currentlyValid = currentKeyPartOfDesiredKey();
  bool should_check_list_end_symbol_append = false;
  string previousKey = getCurrentJsonPathQueryKey();
  string finalResult = "";
  if (streamToken.token_type == JsonEventType::KEY_EVENT) {
    ignoreEventFlag = ignoreEventFlag || !currentKeyPartOfDesiredKey();
    push_string_key(streamToken.value);
  }
  if (streamToken.token_type == VALUE_EVENT) {
    if (getPartOfValue() == "object") {
      pop_key();
      shouldAddThisEvent = true;
    } else if (getPartOfValue() == "list") {
      set_new_value_added_in_list();
    }
  }
  if (streamToken.token_type == JsonEventType::OBJECT_LIST_EVENT) {
    if (streamToken.value == "list started") {
      setNewListStarted();
      setPartOfValue("list");
    } else if (streamToken.value == "list ended") {
      pop_part_of_value();
      if (isLastKeyInKeyStackIsIndex()) {
        pop_key();
      }
      if (getPartOfValue() == "object") {
        pop_key();
      }
      shouldAddThisEvent = true;
      should_check_list_end_symbol_append = true;
    } else if (streamToken.value == "object started") {
      if (getPartOfValue() == "list") {
        set_new_value_added_in_list();
      }
      setPartOfValue("object");
    } else if (streamToken.value == "object ended") {
      set_last_object_ended();
      shouldAddThisEvent = true;
    }
  }
  bool fg = currentKeyPartOfDesiredKey();
  if (fg && !ignoreEventFlag) {
    string currentKey = getCurrentJsonPathQueryKey();
    addTokenInDesiredResult(streamToken, currentKey + "");
  } else if (!fg && shouldAddThisEvent && currentlyValid) {
    addTokenInDesiredResult(streamToken, previousKey, should_check_list_end_symbol_append);
  }
  if (streamToken.token_type == Document_END) {
    for (auto it = jsonPathQueryResultsMap.begin(); it!=jsonPathQueryResultsMap.end();it++) {
      string key = it->first;
      string value = it->second;
      value = removeLastDelimeterIfNeeded(key);
      json_stream_parser.stop_emitting_event = true;
      if (finalResult.length()) finalResult.push_back(',');
      finalResult.append(value);
    }
    if (multiResultExist) {
      finalResult = "[" + finalResult + "]";
    }
    std::cout << "Got value of desired key final result: " << endl<< finalResult << endl;
  }
}

void addKey(bool list_index, string val) {
  if (list_index) {
    KeyClass keyClass(val == "*" ? -2 : stoi(val));
    jsonPathQueryTokenized.push_back(keyClass);
  } else {
    KeyClass keyClass(val, true);
    jsonPathQueryTokenized.push_back(keyClass);
  }
}
void processJsonPathQuery(string json_path_query) {
  string curr = "";
  bool list_index = false;
  int last_list_end = -1, last_dot_index = -1;

  for (int i = 0; i < json_path_query.size(); i++) {
    if (json_path_query[i] == '.') {
      if (i - 1 != last_list_end) {
        addKey(list_index, curr);
        last_dot_index = i;
      }
      curr = "";
    } else if (json_path_query[i] == '[') {
      if (i - 1 != last_dot_index && i - 1 != last_list_end) addKey(list_index, curr);
      list_index = true;
      curr = "";
    } else if (json_path_query[i] == ']') {
      addKey(list_index, curr);
      list_index = false;
      curr = "";
      last_list_end = i;
    } else {
      curr.push_back(json_path_query[i]);
    }
  }
  if (curr.length()) addKey(list_index, curr);
  
  for (int i = 0; i < jsonPathQueryTokenized.size(); i++) {
    multiResultExist = multiResultExist || (jsonPathQueryTokenized[i].any_key || jsonPathQueryTokenized[i].any_index);
  }
}

int main(int argc, char** argv) {
  json_stream_parser = Json_stream_parser();
  string fileName(argv[1]);
  string json_path_query(argv[2]);
  fileName = "tests/Json files/" + fileName;
  KeyClass keyClass("$", true);
  key_stack.push_back(keyClass);
  processJsonPathQuery(json_path_query);
  json_stream_parser.setEventHandler(handleEvent);
  json_stream_parser.start_json_streaming(fileName);
}