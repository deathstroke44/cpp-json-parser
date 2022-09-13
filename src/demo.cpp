// code beautified using https://codebeautify.org/cpp-formatter-beautifier and https://formatter.org/cpp-formatter
#include "library/code.hh"
using namespace std;
class KeyClass {
 public:
  bool index_key = false;
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
    index_key = true;
    this->index = 0;
    any_key = (key.length() == 0 || key=="*");
  };
};
vector<KeyClass> key_stack;
vector<int> list_index_stack;
vector<string> part_of_stack;
string desired_key = "skills";
JsonStreamEvent<string> current_event;
map<string, string> traversedJsonMap;
Json_stream_parser json_stream_parser;
vector<KeyClass> desired_key_list;
string getDummyKey() {
  string str = "";
  for (int i = 0; i < desired_key_list.size() && i < key_stack.size(); i++) {
    KeyClass kc = key_stack[i];
    if (!kc.index_key) {
      str = str + "." + "[" + to_string(kc.index) + "]";
    } else {
      str = str + "." + kc.key;
    }
  }
  return str;
}
string getKeyValue(string key) {
  return traversedJsonMap[key];
}
void putKeyValue(string key, string value) { traversedJsonMap[key] = value; }
void remove_delimeter_not_exists(const StreamToken streamToken,
                                 string currentKey) {
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
string remove_last_delimeter(string key) {
  string traversedJson = getKeyValue(key);
  if (traversedJson.size() > 0 &&
      traversedJson[traversedJson.size() - 1] == ',') {
    traversedJson.pop_back();
    putKeyValue(key, traversedJson);
  }
  return traversedJson;
}
void addEventInDesiredResult(const StreamToken streamToken, string currentKey) {
  remove_delimeter_not_exists(streamToken, currentKey);
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
      traversedJson.push_back(']');
      traversedJson.push_back(',');
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
  if (desired_key_list.size() <= key_stack.size()) {
    for (int i = 0; i < desired_key_list.size(); i++) {
      KeyClass kcC = key_stack[i];
      KeyClass kcD = desired_key_list[i];
      if (kcD.index_key != kcC.index_key) return false;
      if (kcD.index_key) {
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
void getProcessedKey(string type) {
  if (key_stack.size()) {
    KeyClass keyClass = key_stack[key_stack.size() - 1];
    if (keyClass.index_key) {
      if (type == "expand") {
      } else if (type == "deflate") {
        key_stack.pop_back();
      }
    } else {
      if (type == "expand") {
      } else if (type == "deflate") {
        key_stack.pop_back();
      }
    }
  }
}
void pop_key(bool need_to_added_ine_event = false) {
  getProcessedKey("deflate");
}
void increment_index_key() {
  if (key_stack.size() && !key_stack[key_stack.size() - 1].index_key) {
    KeyClass keyClass = key_stack[key_stack.size() - 1];
    getProcessedKey("deflate");
    keyClass.index++;
    key_stack.push_back(keyClass);
    getProcessedKey("expand");
  }
}

void push_string_key(string key) {
  key_stack.push_back(KeyClass(key, true));
  getProcessedKey("expand");
}

void push_index_key(int index) {
  key_stack.push_back(KeyClass(index));
  getProcessedKey("expand");
}

string get_part_of_value() {
  if (part_of_stack.size() == 0) return "";
  return part_of_stack[part_of_stack.size() - 1];
}

void set_new_list_started() {
  if (get_part_of_value() == "list") {
    if (list_index_stack.size() > 0) {
      list_index_stack[list_index_stack.size() - 1]++;
      increment_index_key();
    }
    list_index_stack.push_back(-1);
    push_index_key(-1);

  } else {
    list_index_stack.push_back(-1);
    push_index_key(-1);
  }
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
    if (list_index_stack.size() > 0) {
      list_index_stack[list_index_stack.size() - 1]++;
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
  bool check_balance_par = false;
  string previousKey = getDummyKey();
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
        set_part_of_value("list");
        set_new_list_started();
      } else if (streamToken.value == "list ended") {
        pop_part_of_value();
        if (list_index_stack.size() > 0) {
          list_index_stack.pop_back();
          pop_key(true);
        }
        if (get_part_of_value() == "object") {
          pop_key(true);
        }
        shouldAddThisEvent = true;
        check_balance_par = true;
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
      string currentKey = getDummyKey();
      addEventInDesiredResult(streamToken, currentKey + "");
    } else if (!fg && shouldAddThisEvent && currentlyValid) {
      addEventInDesiredResult(streamToken, previousKey);
      if (check_balance_par) {
        string traversedJson = getKeyValue(previousKey);
        int cnt = 0;
        for (int i = 0; i < traversedJson.size(); i++) {
          if (traversedJson[i] == ']') cnt++;
          if (traversedJson[i] == '[') cnt--;
        }
        if (cnt == 1) {
          traversedJson.pop_back();
          traversedJson.pop_back();
          putKeyValue(previousKey, traversedJson);
        }
      }
    }
  }
  string finalResult = "";
  if (streamToken.token_type == Document_END) {
    for (auto it : traversedJsonMap) {
      string key = it.first;
      string value = it.second;
      value = remove_last_delimeter(key);
      json_stream_parser.stop_emitting_event = true;
      if (finalResult.length()) finalResult.push_back(',');
      finalResult.append(value);
    }
    if (multiResultExist) {
      finalResult = "[" + finalResult;
      finalResult.push_back(']');
    }

    std::cout << "Got value of desired key final result: " << endl
              << finalResult << endl;
  }
}

void addKey(bool list_index, string val) {
  if (list_index) {
    KeyClass keyClass(val == "*" ? -2 : stoi(val));
    desired_key_list.push_back(keyClass);
  } else {
    KeyClass keyClass(val, true);
    desired_key_list.push_back(keyClass);
  }
}
void splitDesiredKey() {
  string curr = "";
  bool list_index = false;
  int last_list_end = -1;
  int last_dot_index = -1;

  for (int i = 0; i < desired_key.size(); i++) {
    if (desired_key[i] == '.') {
      if (i - 1 != last_list_end) {
        addKey(list_index, curr);
        last_dot_index = i;
      }
      curr = "";
    } else if (desired_key[i] == '[') {
      if (i - 1 != last_dot_index) addKey(list_index, curr);
      list_index = true;
      curr = "";
    } else if (desired_key[i] == ']') {
      addKey(list_index, curr);
      list_index = false;
      curr = "";
      last_list_end = i;
    } else {
      curr.push_back(desired_key[i]);
    }
  }
  if (curr.length()) {
    addKey(list_index, curr);
  }
  for (int i = 0; i < desired_key_list.size(); i++) {
    if (desired_key_list[i].any_key || desired_key_list[i].any_index) {
      multiResultExist = true;
    }
  }
}

int main(int argc, char** argv) {
  json_stream_parser = Json_stream_parser();
  string _fileName(argv[1]);
  string _desired_key(argv[2]);
  desired_key = _desired_key;
  string fileName = "tests/Json files/" + _fileName;
  KeyClass keyClass("$", true);
  key_stack.push_back(keyClass);
  splitDesiredKey();
  json_stream_parser.setEventHandler(handleEvent);
  json_stream_parser.start_json_streaming(fileName);
}