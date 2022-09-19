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
vector<string> part_of_stack;
JsonStreamEvent<string> current_event;
map<string, string> jsonPathQueryResultsMap;
Json_stream_parser json_stream_parser;
vector<KeyClass> json_path_query_tokenized;
string getCurrentJsonPathQueryKey() {
  string str = "";
  for (int i = 0; i < json_path_query_tokenized.size() && i < key_stack.size(); i++) {
    KeyClass kc = key_stack[i];
    if (!kc.is_string_key) {
      str = str + "." + "[" + to_string(kc.index) + "]";
    } else {
      str = str + "." + kc.key;
    }
  }
  return str;
}
string getKeyValue(string key) {
  return jsonPathQueryResultsMap[key];
}
bool isLastKeyInKeyStackIsIndex() { return key_stack.size() && !key_stack[key_stack.size()-1].is_string_key;}
void putKeyValue(string key, string value) { jsonPathQueryResultsMap[key] = value; }
void removeDelimeterIfNeeded(const StreamToken streamToken, string currentKey) {
  string traversedJson = getKeyValue(currentKey);
  bool flag = streamToken.token_type == JsonEventType::OBJECT_LIST_EVENT &&
              ((streamToken.value == "list ended") ||
               (streamToken.value == "object ended"));
  if (flag && traversedJson.size() > 0 &&
      traversedJson[traversedJson.size() - 1] == ',') {
    traversedJson.pop_back();
  }
  putKeyValue(currentKey, traversedJson);
}
string removeLastDelimeterIfNeeded(string key) {
  string traversedJson = getKeyValue(key);
  if (traversedJson.size() > 0 &&
      traversedJson[traversedJson.size() - 1] == ',') {
    traversedJson.pop_back();
    putKeyValue(key, traversedJson);
  }
  return traversedJson;
}
void addTokenInDesiredResult(const StreamToken streamToken, string currentKey, bool listEndTagCheck=false) {
  removeDelimeterIfNeeded(streamToken, currentKey);
  string traversedJson = getKeyValue(currentKey);
  if (streamToken.token_type == JsonEventType::KEY_EVENT) {
    traversedJson += "\"" + streamToken.value + "\"" + " : ";
  }
  if (streamToken.token_type == VALUE_EVENT) {
    if (streamToken.token_sub_type == JsonEventType::STRING_EVENT) {
      traversedJson += "\"" + streamToken.value + "\",";
    } else {
      traversedJson += streamToken.value + ",";
    }
  }
  if (streamToken.token_type == JsonEventType::OBJECT_LIST_EVENT) {
    if (streamToken.value == "list started") {
      traversedJson.push_back('[');
    } else if (streamToken.value == "list ended") {
      if (!listEndTagCheck || (key_stack.size()+1==json_path_query_tokenized.size() && json_path_query_tokenized[json_path_query_tokenized.size()-1].is_string_key)) {
        traversedJson.push_back(']');
        traversedJson.push_back(',');
      }
    } else if (streamToken.value == "object started") {
      traversedJson.push_back('{');
    } else if (streamToken.value == "object ended") {
      traversedJson.push_back('}');
      traversedJson.push_back(',');
    }
  }
  putKeyValue(currentKey, traversedJson);
}
bool current_key_part_of_desired_key() {
  if (json_path_query_tokenized.size() <= key_stack.size()) {
    for (int i = 0; i < json_path_query_tokenized.size(); i++) {
      KeyClass kcC = key_stack[i];
      KeyClass kcD = json_path_query_tokenized[i];
      if (kcD.is_string_key != kcC.is_string_key) return false;
      if (kcD.is_string_key) {
        if (!(kcD.any_key || kcD.key == kcC.key)) {
          return false;
        }
      } else {
        if (!((kcD.any_index && kcC.index != -1) || kcD.index == kcC.index)) {
          return false;
        }
      }
    }
  } else
    return false;
  return true;
}

void pop_key(bool need_to_added_ine_event = false) {
  if (key_stack.size()) {
    key_stack.pop_back();
  }
}
void increment_index_key() {
  if (isLastKeyInKeyStackIsIndex()) key_stack[key_stack.size() - 1].index++;
}

void push_string_key(string key) {
  key_stack.push_back(KeyClass(key, true));
}

void push_index_key(int index) {
  key_stack.push_back(KeyClass(index));
}

string get_part_of_value() {
  if (part_of_stack.size() == 0) return "";
  return part_of_stack[part_of_stack.size() - 1];
}

void set_new_list_started() {
  if (get_part_of_value() == "list" && isLastKeyInKeyStackIsIndex()) {
    increment_index_key();
  }
  push_index_key(-1);
}
void pop_part_of_value() {
  if (part_of_stack.size() > 0) {
    part_of_stack.pop_back();
  }
}

void set_last_object_ended() {
  pop_part_of_value();
  if (get_part_of_value() == "object") {
    pop_key(true);
  }
}

void set_new_value_added_in_list() {
  if (get_part_of_value() == "list") {
    if (isLastKeyInKeyStackIsIndex()) {
      increment_index_key();
    }
  }
}

void set_part_of_value(string value) { part_of_stack.push_back(value); }

bool multiResultExist = false;

void handleEvent(const JsonStreamEvent<string>& event) {
  current_event = event;
  StreamToken streamToken = event.getStreamToken();
  bool ignoreEventFlag = false;
  bool shouldAddThisEvent = false;
  bool currentlyValid = current_key_part_of_desired_key();
  bool should_check_list_end_symbol_append = false;
  string previousKey = getCurrentJsonPathQueryKey();
  {
    if (streamToken.token_type == JsonEventType::KEY_EVENT) {
      if (!current_key_part_of_desired_key()) {
        ignoreEventFlag = true;
      }
      push_string_key(streamToken.value);
    }
    if (streamToken.token_type == VALUE_EVENT) {
      if (get_part_of_value() == "object") {
        pop_key(true);
        shouldAddThisEvent = true;
      } else if (get_part_of_value() == "list") {
        set_new_value_added_in_list();
      }
    }
    if (streamToken.token_type == JsonEventType::OBJECT_LIST_EVENT) {
      if (streamToken.value == "list started") {
        set_new_list_started();
        set_part_of_value("list");
      } else if (streamToken.value == "list ended") {
        pop_part_of_value();
        if (isLastKeyInKeyStackIsIndex()) {
          pop_key(true);
        }
        if (get_part_of_value() == "object") {
          pop_key(true);
        }
        shouldAddThisEvent = true;
        should_check_list_end_symbol_append = true;
      } else if (streamToken.value == "object started") {
        if (get_part_of_value() == "list") {
          set_new_value_added_in_list();
        }
        set_part_of_value("object");
      } else if (streamToken.value == "object ended") {
        set_last_object_ended();
        shouldAddThisEvent = true;
      }
    }
    bool fg = current_key_part_of_desired_key();
    if (fg && !ignoreEventFlag) {
      string currentKey = getCurrentJsonPathQueryKey();
      addTokenInDesiredResult(streamToken, currentKey + "");
    } else if (!fg && shouldAddThisEvent && currentlyValid) {
      addTokenInDesiredResult(streamToken, previousKey, should_check_list_end_symbol_append);
    }
  }
  string finalResult = "";
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
    json_path_query_tokenized.push_back(keyClass);
  } else {
    KeyClass keyClass(val, true);
    json_path_query_tokenized.push_back(keyClass);
  }
}
void processJsonPathQuery(string json_path_query) {
  string curr = "";
  bool list_index = false;
  int last_list_end = -1;
  int last_dot_index = -1;

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
  
  for (int i = 0; i < json_path_query_tokenized.size(); i++) {
    multiResultExist = multiResultExist || (json_path_query_tokenized[i].any_key || json_path_query_tokenized[i].any_index);
  }
}

int main(int argc, char** argv) {
  json_stream_parser = Json_stream_parser();
  string _fileName(argv[1]);
  string json_path_query(argv[2]);
  string fileName = "tests/Json files/" + _fileName;
  KeyClass keyClass("$", true);
  key_stack.push_back(keyClass);
  processJsonPathQuery(json_path_query);
  json_stream_parser.setEventHandler(handleEvent);
  json_stream_parser.start_json_streaming(fileName);
}