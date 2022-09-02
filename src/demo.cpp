#include "code.hh"
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
pair<JsonEventType,string> last_event = make_pair(JsonEventType::NULL_EVENT, "");
int key_found = 0; // 0 not found , 1 traversing, 2 ended
string current_key="";
string desired_key = "skills"; // skills.nested1.nested3.key2  skills.lang [0].id
Event<JsonEventType> current_event;
Event<JsonEventType> last_desired_event;
string traversedJson = "";

void remove_delimeter_not_exists(const Event<JsonEventType>& event) {
    bool flag = event.type() == JsonEventType::OBJECT_LIST_EVENT && ((event.getName() == "list ended") ||
    (event.getName() == "object ended"));
    if (flag && traversedJson.size()>0 && traversedJson[traversedJson.size()-1]==',') {
        traversedJson.pop_back();
    }

}
void remove_last_delimeter() {
    if (traversedJson.size()>0 && traversedJson[traversedJson.size()-1]==',') {
        traversedJson.pop_back();
    }

}

void addEventInDesiredResult(const Event<JsonEventType>& event) {
    remove_delimeter_not_exists(event);
    // cout<<"traversed json before: "<<traversedJson<<endl;
    if(event.type() == JsonEventType::KEY_EVENT) { 
        traversedJson += "\""+event.getName()+ "\""+" : ";
        std::cout << "EMITTED Event TYPE: KEY " << event.getName() << std::endl;
    }
    if(event.type() == JsonEventType::STRING_EVENT) { 
        traversedJson += "\""+event.getName()+ "\",";
        std::cout << "EMITTED Event TYPE: String " << event.getName() << std::endl;}
    if(event.type() == JsonEventType::BOOL_EVENT) { 
        traversedJson += event.getName()+ ","; 
        std::cout << "EMITTED Event TYPE: BOOL " << event.getName() << std::endl;}
    if(event.type() == JsonEventType::NULL_EVENT) { 
        traversedJson += event.getName()+ ","; 
        std::cout << "EMITTED Event TYPE: NULL " << event.getName() << std::endl;}
    if(event.type() == JsonEventType::INTEGER_EVENT) {  
        traversedJson += event.getName()+ ",";
        std::cout << "EMITTED Event TYPE: INTEGER " << event.getName() << std::endl;}
    if(event.type() == JsonEventType::FLOAT_EVENT) {  
        traversedJson += event.getName()+ ",";
        std::cout << "EMITTED Event TYPE: FLOAT " << event.getName() << std::endl;}
    if(event.type() == JsonEventType::EXPONENT_EVENT) {  
        traversedJson += event.getName()+ ",";
        std::cout << "EMITTED Event TYPE: EXPONENT " << event.getName() << std::endl;}
    if(event.type() == JsonEventType::OBJECT_LIST_EVENT) {
        if (event.getName() == "list started") {
            traversedJson.push_back('[');
        }
        else if (event.getName() == "list ended") {
            traversedJson.push_back(']');
            traversedJson.push_back(',');
        }
        else if (event.getName() == "object started") {

            traversedJson.push_back('{');

        }
        else if (event.getName() == "object ended") {
            traversedJson.push_back('}');
            traversedJson.push_back(',');
            
        }
    }
    if(event.type() == JsonEventType::Document_END) { 
        std::cout << "EMITTED Event TYPE: Full Document Read Completed " << std::endl;}
    cout<<"---------------------------------------------------------------------------------"<<endl;

    // cout<<"traversed json after: "<<traversedJson<<endl;
    last_desired_event = event;

}

bool current_key_part_of_desired_key() {
    return current_key.rfind(desired_key,0) == 0;
}


void getProcessedKey(string type) {
    // type expand, updated, deflate
    // cout<<"Current key before: "<<type<<" "<<current_key<<endl;
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
    // cout<<"Current key after: "<<type<<" "<<current_key<<endl;
}



void pop_key(bool need_to_added_ine_event = false) {
    getProcessedKey("deflate");
    if (!current_key_part_of_desired_key() && key_found==1) {
        key_found = 2;
        if (need_to_added_ine_event) {
            addEventInDesiredResult(current_event);
        }
    }

}


void increment_index_key() {
    // cout<<"Code debug: "<<key_stack.size()<<endl;
    if (key_stack.size() && !key_stack[key_stack.size()-1].index_key) {
        KeyClass keyClass = key_stack[key_stack.size()-1];
        getProcessedKey("deflate");
        keyClass.index++;
        key_stack.push_back(keyClass);
        getProcessedKey("expand");
        if (current_key_part_of_desired_key() && key_found==0) {
            key_found = 1;
            addEventInDesiredResult(current_event);
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
        addEventInDesiredResult(current_event);
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


void print_debug(string event) {
    // cout<<event<<endl;
    // for(int i=0;i<list_index_stack.size();i++) {
    //     cout<<list_index_stack[i]<<" ";
    // }
    // cout<<endl;
    // for(int i=0;i<part_of_stack.size();i++) {
    //     cout<<part_of_stack[i]<<" ";
    // }
    // cout<<endl;

    // for(int i=0;i<key_stack.size();i++) {
    //     cout<<key_stack[i].index_key<<" "<<key_stack[i].index<<" "<<key_stack[i].key<<" |";
    // }
    // cout<<"\n---------------------------------------------------------------------------------------------------------------------"<<endl;
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






void handleEvent(const Event<JsonEventType>& event) {
    current_event = event;
    // handle event as your need. I have just added this code for testing and demo purpose.
    int prev_key_found = key_found;
    if(event.type() == JsonEventType::Document_END) { 
        remove_last_delimeter();
        std::cout << "EMITTED Event TYPE: Full Document Read Completed " << endl<<traversedJson<<endl;
    }
    if (key_found == 0 || key_found ==1)
    {
        if(event.type() == JsonEventType::KEY_EVENT) { 
            push_string_key(event.getName());
            print_debug(event.getName());
        }
        if (event.type() == JsonEventType::STRING_EVENT || 
        event.type() == JsonEventType::BOOL_EVENT ||
        event.type() == JsonEventType::NULL_EVENT ||
        event.type() == JsonEventType::INTEGER_EVENT ||
        event.type() == JsonEventType::FLOAT_EVENT ||
        event.type() == JsonEventType::EXPONENT_EVENT) {
            handle_value_found();
            print_debug(event.getName());

        }
        if(event.type() == JsonEventType::OBJECT_LIST_EVENT) {
            if (event.getName() == "list started") {
                set_part_of_value("list");
                set_new_list_started();
            }
            else if (event.getName() == "list ended") {
                set_last_list_ended();
            }
            else if (event.getName() == "object started") {
                if (get_part_of_value() == "list") {
                    set_new_value_added_in_list();
                }
                set_part_of_value("object");
            }
            else if (event.getName() == "object ended") {
                set_last_object_ended();
            }
            print_debug(event.getName());

        }
        if(event.type() == JsonEventType::Document_END) { std::cout << "EMITTED Event TYPE: Full Document Read Completed " << std::endl;}
        if (key_found == 1) {
            if (prev_key_found == 0) {

            }
            else if (prev_key_found == 1) {
                addEventInDesiredResult(current_event);
            }
        }
        else if(key_found == 1) {
            addEventInDesiredResult(current_event);
        }
        else if(key_found == 2) {
            
        }
        
        // cout<<"---------------------------------------------------------------------------------"<<endl;
    }
    if (key_found == 1) {

    }
    last_event = make_pair(event.type(), event.getName());
}

int main() {
    Json_stream_parser json_stream_parser = Json_stream_parser();
    string fileName = "code-test.json"; // code-test.json large-file.json
    // subscribe to the events those are needed for your purpose
    // Subscribing to all events is not necessary. 
    // There is no constraint that you have to use same fuction for all events
    {
        json_stream_parser.eventDispatcher.subscribe( JsonEventType::KEY_EVENT, handleEvent );
        json_stream_parser.eventDispatcher.subscribe( JsonEventType::STRING_EVENT, handleEvent );
        json_stream_parser.eventDispatcher.subscribe( JsonEventType::BOOL_EVENT, handleEvent );
        json_stream_parser.eventDispatcher.subscribe( JsonEventType::NULL_EVENT, handleEvent );
        json_stream_parser.eventDispatcher.subscribe( JsonEventType::INTEGER_EVENT, handleEvent );
        json_stream_parser.eventDispatcher.subscribe( JsonEventType::FLOAT_EVENT, handleEvent );
        json_stream_parser.eventDispatcher.subscribe( JsonEventType::EXPONENT_EVENT, handleEvent );
        json_stream_parser.eventDispatcher.subscribe( JsonEventType::OBJECT_LIST_EVENT, handleEvent );
        json_stream_parser.eventDispatcher.subscribe( JsonEventType::Document_END, handleEvent );
    }

    json_stream_parser.start_json_streaming(fileName);
}