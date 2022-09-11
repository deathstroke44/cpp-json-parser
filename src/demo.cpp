#include "library/code.hh"
using namespace std;
class KeyClass {
    public:
        bool index_key = false;
        int index;
        string key;
        KeyClass() = default;
        KeyClass(int index) {this->index = index;};
        KeyClass(string key, bool dummy) {this->key = key; index_key = true; this->index = 0;};
};
vector<KeyClass> key_stack;
vector<int> list_index_stack;
vector<string> part_of_stack;
int key_found = 0; // 0 not found , 1 traversing, 2 ended
string current_key="";
string desired_key = "skills"; // skills.nested1.nested3.key2  skills.lang [0].id
JsonStreamEvent<string> current_event;
string traversedJson = "";
Json_stream_parser json_stream_parser;
void remove_delimeter_not_exists(const StreamToken streamToken) {
    bool flag = streamToken.token_type == JsonEventType::OBJECT_LIST_EVENT && ((streamToken.value == "list ended") ||
    (streamToken.value == "object ended"));
    if (flag && traversedJson.size()>0 && traversedJson[traversedJson.size()-1]==',') {
        traversedJson.pop_back();
    }
}
void remove_last_delimeter() {
    if (traversedJson.size()>0 && traversedJson[traversedJson.size()-1]==',') {
        traversedJson.pop_back();
    }
}
void addEventInDesiredResult(const StreamToken streamToken) {
    remove_delimeter_not_exists(streamToken);
    if(streamToken.token_type == JsonEventType::KEY_EVENT) { traversedJson += "\""+streamToken.value+ "\""+" : ";}
    if(streamToken.token_type == VALUE_EVENT) {
        if(streamToken.token_sub_type == JsonEventType::STRING_EVENT) { traversedJson += "\""+streamToken.value+ "\",";}
        else {
            traversedJson += streamToken.value+ ",";
        }
    }
    if(streamToken.token_type == JsonEventType::OBJECT_LIST_EVENT) {
        if (streamToken.value == "list started") {traversedJson.push_back('[');}
        else if (streamToken.value == "list ended") {
            traversedJson.push_back(']');
            traversedJson.push_back(',');
        }
        else if (streamToken.value == "object started") {traversedJson.push_back('{');}
        else if (streamToken.value == "object ended") {
            traversedJson.push_back('}');
            traversedJson.push_back(',');
        }
    }
}
bool current_key_part_of_desired_key() { return current_key.rfind(desired_key,0) == 0;}
void getProcessedKey(string type) {
    if (key_stack.size()){
        KeyClass keyClass = key_stack[key_stack.size()-1];
        if (keyClass.index_key) {
            if (type == "expand") {
                if (key_stack.size()>1) {
                    current_key = current_key + '.'; 
                }
                current_key = current_key + keyClass.key;
            }
            else if (type == "deflate") {
                int last_key_length = keyClass.key.size();
                if (key_stack.size()>1) {
                    last_key_length++; 
                }
                current_key = current_key.substr(0, current_key.size() - last_key_length);
                key_stack.pop_back();
            }
        }
        else {
            if (type == "expand") {
                string new_key ="["+to_string(keyClass.index)+"]";
                current_key = current_key + new_key;
            }
            else if (type == "deflate") {
                string new_key ="["+to_string(keyClass.index)+"]";
                int last_key_length = new_key.size();
                current_key = current_key.substr(0, current_key.size() -last_key_length);
                key_stack.pop_back();
            }
        }
    }
}
void pop_key(bool need_to_added_ine_event = false) {
    getProcessedKey("deflate");
    if (!current_key_part_of_desired_key() && key_found==1) {
        key_found = 2;
        StreamToken current_stream_token = current_event.getStreamToken();
        if (current_stream_token.token_sub_type == OBJECT_LIST_EVENT && current_stream_token.value == "list ended") {
            need_to_added_ine_event = !(traversedJson.length() && traversedJson[0] != '[');
        }
        if (need_to_added_ine_event) {
            addEventInDesiredResult(current_stream_token);
        }
    }
}
void increment_index_key() {
    if (key_stack.size() && !key_stack[key_stack.size()-1].index_key) {
        KeyClass keyClass = key_stack[key_stack.size()-1];
        getProcessedKey("deflate");
        keyClass.index++;
        key_stack.push_back(keyClass);
        getProcessedKey("expand");
        if (current_key_part_of_desired_key() && key_found==0) {
            key_found = 1;
            addEventInDesiredResult(current_event.getStreamToken());
        }
        else if (!current_key_part_of_desired_key() && key_found==1) {
            key_found = 2;
        }
    }
}

void push_string_key(string key) {
    key_stack.push_back(KeyClass(key, true));
    getProcessedKey("expand");
    if (current_key_part_of_desired_key() && key_found==0) {
        key_found = 1;
    }
}

void push_index_key(int index) {
    key_stack.push_back(KeyClass(index));
    getProcessedKey("expand");
    if (current_key_part_of_desired_key() && key_found==0) {
        key_found = 1;
        addEventInDesiredResult(current_event.getStreamToken());
    }
    if (!current_key_part_of_desired_key() && key_found==1) {
        key_found = 2;
    }
}

string get_part_of_value() {
    if (part_of_stack.size() == 0) return "";
    return part_of_stack[part_of_stack.size()-1];
}

void set_new_list_started() {
    if (get_part_of_value() == "list") {
        if (list_index_stack.size()>0) {
            list_index_stack[list_index_stack.size()-1]++;
            increment_index_key();
        }
        list_index_stack.push_back(-1);
        push_index_key(-1);

    }
    else {
        list_index_stack.push_back(-1);
        push_index_key(-1);
    }
}
void pop_part_of_value() {
    if (part_of_stack.size()>0) {
        part_of_stack.pop_back();
    }
}
void set_last_list_ended() {
    pop_part_of_value();
    if (list_index_stack.size()>0) {
        list_index_stack.pop_back();
        pop_key(true);
    }
    if (get_part_of_value() == "object") {
        pop_key(true);
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
        if (list_index_stack.size()>0) {
            list_index_stack[list_index_stack.size()-1]++;
            increment_index_key();
        }
    }
}

void set_part_of_value(string value) {
    part_of_stack.push_back(value);
}

void handle_value_found() {
    if (get_part_of_value() == "object") {
        pop_key(true);
    }
    else if (get_part_of_value() == "list") {
        set_new_value_added_in_list();
    }
}

void handleEvent(const JsonStreamEvent<string>& event) {
    current_event = event;
    // handle event as your need. I have just added this code for testing and demo purpose.
    int prev_key_found = key_found;
    StreamToken streamToken = event.getStreamToken();
    if (key_found == 0 || key_found ==1)
    {
        if(streamToken.token_type == JsonEventType::KEY_EVENT) { push_string_key(streamToken.value);}
        if (streamToken.token_type == VALUE_EVENT) {
            handle_value_found();

        }
        if(streamToken.token_type == JsonEventType::OBJECT_LIST_EVENT) {
            if (streamToken.value == "list started") {
                set_part_of_value("list");
                set_new_list_started();
            }
            else if (streamToken.value == "list ended") {
                set_last_list_ended();
            }
            else if (streamToken.value == "object started") {
                if (get_part_of_value() == "list") {
                    set_new_value_added_in_list();
                }
                set_part_of_value("object");
            }
            else if (streamToken.value == "object ended") {
                set_last_object_ended();
            }

        }
        if(key_found == 1 && prev_key_found == 1) {
            addEventInDesiredResult(current_event.getStreamToken());
        }
        else if(key_found == 2 && prev_key_found != 2) {
            remove_last_delimeter();
            json_stream_parser.stop_emitting_event = true;
            std::cout << "Got value of desired key: " << desired_key<<endl<<traversedJson<<endl;
        }
    }
}

int main(int argc, char** argv) {
    json_stream_parser = Json_stream_parser();
    string _fileName(argv[1]);
    string _desired_key(argv[2]);
    desired_key= _desired_key;
    string fileName = "tests/Json files/"+_fileName;
    // code-test.json large-file.json
    // subscribe to the events those are needed for your purpose
    // Subscribing to all events is not necessary. 
    // There is no constraint that you have to use same fuction for all events
    {
        json_stream_parser.setEventHandler(handleEvent);
    }

    json_stream_parser.start_json_streaming(fileName);
}