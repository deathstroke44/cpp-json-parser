#include<bits/stdc++.h>
#include "Dispatcher.hh"
#include "Event.hh"
using namespace std;
#define sfi(a) scanf("%d",&a)
#define sfl(a) scanf("%lld",&a)
#define sff(a) scanf("%lf",&a)
#define sfs(a) scanf("%s",&a)
#define pf printf
#define MAX 110
#define mymax(a,b,c) max(a,max(b,c))
#define mymin(a,b,c) min(a,min(b,c))
#define mymiddle(a,b,c) a+b+c-max(a,max(b,c))-min(a,min(b,c))
#define check(n, pos) (n & (1<<pos))
#define sq(x) ((x)*(x))
typedef long long int li;




class JsonEvent : public Event<JsonEventType>
{
    public:
    JsonEvent() : Event<JsonEventType>(JsonEventType::KEY_EVENT, "JsonEvent"){};
    JsonEvent(JsonEventType jsonEventType, string value) : Event<JsonEventType>(jsonEventType,  value){};
    virtual ~JsonEvent() = default;
};





void onEvent1(const Event<JsonEventType>& event){
  if(event.type() == JsonEventType::KEY_EVENT) { std::cout << "EMITTED Event TYPE: KEY " << event.getName() << std::endl;}
  if(event.type() == JsonEventType::STRING_EVENT) { std::cout << "EMITTED Event TYPE: String " << event.getName() << std::endl;}
  if(event.type() == JsonEventType::BOOL_EVENT) { std::cout << "EMITTED Event TYPE: BOOL " << event.getName() << std::endl;}
  if(event.type() == JsonEventType::NULL_EVENT) { std::cout << "EMITTED Event TYPE: NULL " << event.getName() << std::endl;}
  if(event.type() == JsonEventType::INTEGER_EVENT) { std::cout << "EMITTED Event TYPE: INTEGER " << event.getName() << std::endl;}
  if(event.type() == JsonEventType::FLOAT_EVENT) { std::cout << "EMITTED Event TYPE: FLOAT " << event.getName() << std::endl;}
  if(event.type() == JsonEventType::EXPONENT_EVENT) { std::cout << "EMITTED Event TYPE: EXPONENT " << event.getName() << std::endl;}
  if(event.type() == JsonEventType::OBJECT_LIST_EVENT) { std::cout << "EMITTED Event TYPE: OBJECT LIST EVENT " << event.getName() << std::endl;}

    cout<<"---------------------------------------------------------------------------------"<<endl;
}


enum FState{
    WHITESPACE_STATE,
    INTEGER_STATE,
    INTEGER_SIGN_STATE,
    INTEGER_0_STATE,
    INTEGER_EXP_STATE,
    INTEGER_EXP_0_STATE,
    FLOATING_POINT_0_STATE,
    FLOATING_POINT_STATE,
    STRING_STATE,
    STRING_ESCAPE_STATE,
    STRING_END_STATE,
    TRUE_1_STATE,
    TRUE_2_STATE,
    TRUE_3_STATE,
    FALSE_1_STATE,
    FALSE_2_STATE,
    FALSE_3_STATE,
    FALSE_4_STATE,
    NULL_1_STATE,
    NULL_2_STATE,
    NULL_3_STATE,
    UNICODE_1_STATE,
    UNICODE_2_STATE,
    UNICODE_3_STATE,
    UNICODE_4_STATE
};
class JsonStreamParser {


vector<string> tokens;
// stack<string> key_stack;
// stack<bool> is_part_of_object;
bool next_string_will_be_key = false;
bool is_completed = false;
int char_code = 0;
FState state = WHITESPACE_STATE;
bool should_advance = true;
bool add_char = false;
FState next_state = WHITESPACE_STATE;
pair<TokenType, string> previous_token;
bool previous_token_un_processed = false;
pair<TokenType, string> latest_token = make_pair(NULL_TOKEN,"");
public:
Dispatcher<JsonEventType> eventDispatcher = Dispatcher<JsonEventType>();
DispatcherV2<string> eventDispatcherV1 = DispatcherV2<string>();
bool stop_emitting_event = false;
string topicName = "jsonStreamTopic";
// void stream_token(pair<TokenType, string> token, bool print_key_end_event);
// void handle_whitespace_state(char &c, int &char_code);

// https://www.json.org/json-en.html
// https://docstore.mik.ua/orelly/xml/pxml/ch04_02.htm



// bool get_is_part_of_object() {
//     return is_part_of_object.top();
// }

// void pop_is_part_of_object() {
//     is_part_of_object.pop();
// }

// void update_is_part_of_object(bool flag) {
//     is_part_of_object.push(flag);
// }


void setEventHandler(void (*func)(const JsonStreamEvent<string>&)) {
    eventDispatcherV1.subscribe(topicName, func);
}

void emitEvent(int eventType, string value, bool listElement) {
    JsonEventType _JsonSubEventType = JsonEventType::STRING_EVENT;
    JsonEventType _JsonEventType = JsonEventType::STRING_EVENT;
    string _value = "";
    if (eventType == 0) {
        // value string
        // cout<<"Json Stream Event: "<<"Value: "<<value<<endl;
        _JsonSubEventType = JsonEventType::STRING_EVENT;
        _JsonEventType = JsonEventType::VALUE_EVENT;
        _value = value;
        
    }
    else if (eventType == 2) {
        // value key
        // cout<<"Json Stream Event: "<<"key: "<<value<<endl;
        _JsonSubEventType = JsonEventType::KEY_EVENT;
        _JsonEventType = _JsonSubEventType;
        _value = value;
    }
    else if (eventType == 3) {
        // list started
        // cout<<"Json Stream Event: "<<"list started"<<endl;
        _JsonSubEventType = JsonEventType::OBJECT_LIST_EVENT;
        _JsonEventType = _JsonSubEventType;
        _value = "list started";
    }
    else if (eventType == 4) {
        // list ended
        // cout<<"Json Stream Event: "<<"list ended"<<endl;
        _JsonSubEventType = JsonEventType::OBJECT_LIST_EVENT;
        _JsonEventType = _JsonSubEventType;
        _value = "list ended";
    }
    else if (eventType == 6) {
        // object started
        // cout<<"Json Stream Event: "<<"object started"<<endl;
        _JsonSubEventType = JsonEventType::OBJECT_LIST_EVENT;
        _JsonEventType = _JsonSubEventType;
        _value = "object started";
    }
    else if (eventType == 7) {
        // object ended
        // cout<<"Json Stream Event: "<<"object ended"<<endl;
        _JsonSubEventType = JsonEventType::OBJECT_LIST_EVENT;
        _JsonEventType = _JsonSubEventType;
        _value = "object ended";
    }
    else if (eventType == 8) {
        // value integer
        // cout<<"Json Stream Event: "<<"Integer Value: "<<stoll(value)<<endl;
        _JsonSubEventType = JsonEventType::INTEGER_EVENT;
        _JsonEventType = JsonEventType::VALUE_EVENT;
        _value = value;
        
    }
    else if (eventType == 9) {
        // value integer
        // cout<<"Json Stream Event: "<<"Float Value: "<<stof(value)<<endl;
        _JsonSubEventType = JsonEventType::FLOAT_EVENT;
        _JsonEventType = VALUE_EVENT;
        _value = value;
        
    }
    else if (eventType == 10) {
        // value integer
        // cout<<"Json Stream Event: "<<"Boolean Value: "<<value<<endl;
        _JsonSubEventType = JsonEventType::BOOL_EVENT;
        _JsonEventType = VALUE_EVENT;
        _value = value;
        
    }
    else if (eventType == 11) {
        // value integer
        // cout<<"Json Stream Event: "<<"Exponent Value: "<<stof(value)<<endl;
        _JsonSubEventType = JsonEventType::EXPONENT_EVENT;
        _JsonEventType = VALUE_EVENT;
        _value = value;
        
    }
    else if (eventType == 12) {
        // value integer
        // cout<<"Json Stream Event: "<<"NULL Value: "<<value<<endl;
        _JsonSubEventType = JsonEventType::NULL_EVENT;
        _JsonEventType = VALUE_EVENT;
        _value = value;
        
    }
    if (eventType==0 || (eventType>=2 && eventType<=12)) {
        JsonStreamEvent jsonStreamEvent(topicName, StreamToken(_JsonEventType, _JsonSubEventType, _value));
        eventDispatcherV1.post(jsonStreamEvent);
    }
}


void process_prev_token(pair<TokenType, string> prev_token, pair<TokenType, string> curr_token) {
    if (curr_token.second == ":") {
        emitEvent(2, prev_token.second, false);
    }
    else {
        emitEvent(0, prev_token.second, false);   
    }
}




void stream_token(pair<TokenType, string> token, bool print_key_end_event) {
    if (previous_token_un_processed) {
        process_prev_token(previous_token, token);
    }
    previous_token_un_processed = false;
    TokenType token_type =  token.first;
    string token_value = token.second;
    previous_token = token;
    if (token_type == OPERATOR_TOKEN) {
        if (token_value == "[") {
            emitEvent(3, "list", false);
        }
        else if (token_value == "]") {
            emitEvent(4, "list", false);
        }
        else if (token_value == "}") {
            emitEvent(7, "object", false);
        }
        else if (token_value == "{") {
            emitEvent(6, "object", false);
        }

    }
    else if(token_type == STRING_TOKEN) {
        previous_token_un_processed = true;
        return;
    }
    else if(token_type == INTEGER_TOKEN) {
        emitEvent(8, token_value, false);
    }
    else if(token_type == FLOAT_TOKEN) {
        emitEvent(9, token_value, false);
    }
    else if(token_type == EXP_TOKEN) {
        emitEvent(11, token_value, false);
    }
    else if(token_type == BOOLEAN_TOKEN) {
        emitEvent(10, token_value, false);
    }
    else if(token_type == NULL_TOKEN) {
        emitEvent(12, token_value, false);
    }

}

void process_new_token(pair<TokenType, string> token) {
    // cout << "\nNew Generated Token: "<< token.first << " "<<token.second<<endl;
    stream_token(token, true);
}


string mergeTokens() {
    string str= "";
    for (int i=0; i<tokens.size(); i++) {
        // cout<<"\nTokens "<< tokens[i]<< endl;
        str.append(tokens.at(i));
    }
    return str;
}





bool is_delimiter(char c) {
    return c == ' ' || c == '{' || c == '}' || c == '[' || c == ']' || c == ':' || c == ',';
}

bool char_range(char c, int _mn, int _mx) {
    return ((c - '0') >= _mn && (c - '0') <= _mx);
}

bool char_range_v1(char c, char _mn, char _mx) {
    return (c >= _mn && c <= _mx);
}

void handle_whitespace_state(char &c, int &char_code) {
    vector<char> common_cases = {'{', '}','[',']',',',':'};
    for (auto i = common_cases.begin(); i != common_cases.end(); ++i) {
        if (*i == c) {
            string s;
            s.push_back(c);
            is_completed = true;
            latest_token = make_pair(OPERATOR_TOKEN, s);
            return;
        }
    }
    switch (c)
    {
        case '\"':
            /* code */
            next_state = STRING_STATE;
            break;
        case '0':
            /* code */
            next_state = INTEGER_0_STATE;
            add_char = true;
            break;
        case '-':
            /* code */
            next_state = INTEGER_SIGN_STATE;
            add_char = true;
            break;
        case 'f':
            /* code */
            next_state = FALSE_1_STATE;
            break;
        case 't':
            /* code */
            next_state = TRUE_1_STATE;
            break;
        case 'n':
            /* code */
            next_state = NULL_1_STATE;
            break;
        
        default:
            // cout<<"code deb 3"<<" "<<char_range(c,1,9)<<endl;
            if (char_range(c,1,9)) {
                next_state = INTEGER_STATE;
                add_char = true;
            }
            break;
    }

}

void handle_integer_state(char &c, int &char_code) {
    // cout<<"Code debug 1: "<<c<<" "<<char_code<<" "<<state<<endl;
    switch (state)
    {
        case INTEGER_0_STATE:{
            if (c == '.') {
                next_state = FLOATING_POINT_0_STATE;
                add_char = true;
            }
            else if (c == 'e' || c == 'E') {
                next_state = INTEGER_EXP_0_STATE;
                add_char = true;
                
            }
            else if (is_delimiter(c)) {
                next_state = WHITESPACE_STATE;
                is_completed = true;
                latest_token = make_pair(INTEGER_TOKEN, "0");
                should_advance = false;
            }
            break;
        }

        case INTEGER_SIGN_STATE:{
            if (c == '0') {
                next_state = INTEGER_0_STATE;
                add_char = true;
            }
            else if (char_range(c,1,9)) {
                next_state = INTEGER_STATE;
                add_char = true;
            }
            break;
        }

        case INTEGER_EXP_0_STATE:{
            if (c == '+' || c == '-' || char_range(c,0,9)) {
                next_state =  INTEGER_EXP_STATE;
                add_char = true;
            }
            break;
        }

        case INTEGER_EXP_STATE:{
            if (char_range(c,0,9)) {
                add_char = true;
            }
            else if (is_delimiter(c)) {
                is_completed = true;
                latest_token = make_pair(EXP_TOKEN, mergeTokens());
                next_state = WHITESPACE_STATE;
                should_advance = false;
            }
            break;
        }

        case INTEGER_STATE: {
            // cout<<"deb 4: "<<is_delimiter(c)<<" "<<mergeTokens();
            if (char_range(c,0,9)) {
                add_char = true;
            }
            else if (is_delimiter(c)) {
                is_completed = true;
                latest_token = make_pair(INTEGER_TOKEN, mergeTokens());
                next_state = WHITESPACE_STATE;
                should_advance = false;
            }
            else if (c == '.') {
                add_char = true;
                next_state = FLOATING_POINT_0_STATE;
            }
            else if (c=='E' || c=='e') {
                add_char = true;
                next_state = INTEGER_EXP_0_STATE;
            }
            break;
        }

        
        
        default:
            break;
    }
    
}

void handle_float_state(char &c, int &char_code) {
    // cout<<"code deb 4: "<<c<<" "<<char_code<<" "<<state<<endl;
    if (state == FLOATING_POINT_STATE) {
        if (char_range(c,0,9)) {
            add_char = true;
        }
        else if (c == 'e' or c == 'E') {
            next_state = INTEGER_EXP_0_STATE;
            add_char = true;
        }
            
        else if (is_delimiter(c)) {
            is_completed = true;
            latest_token = make_pair(FLOAT_TOKEN, mergeTokens());
            next_state = WHITESPACE_STATE;
            should_advance = false;
        }
            
    }
    else if (state == FLOATING_POINT_0_STATE) {
        if (char_range(c,0,9)) {
            next_state = FLOATING_POINT_STATE;
            add_char = true;
        }
    }   
}

void handle_true_state(char &c, int &char_code) {
    if (state == TRUE_1_STATE && c =='r') {
        next_state = TRUE_2_STATE;
    }
    if (state == TRUE_2_STATE && c =='u') {
        next_state = TRUE_3_STATE;
    }
    if (state == TRUE_3_STATE && c =='e') {
        next_state = WHITESPACE_STATE;
        is_completed = true;
        latest_token = make_pair(BOOLEAN_TOKEN, "true");
    }
}

void handle_false_state(char &c, int &char_code) {
    if (state == FALSE_1_STATE && c =='a') {
        next_state = FALSE_2_STATE;
    }
    else if (state == FALSE_2_STATE && c =='l') {
        next_state = FALSE_3_STATE;
    }
    else if (state == FALSE_3_STATE && c =='s') {
        next_state = FALSE_4_STATE;
    }
    else if (state == FALSE_4_STATE && c =='e') {
        next_state = WHITESPACE_STATE;
        is_completed = true;
        latest_token = make_pair(BOOLEAN_TOKEN, "false");
    }

    
}

void handle_null_state(char &c, int &char_code) {
    if (state == NULL_1_STATE && c =='u') {
        next_state = NULL_2_STATE;
    }
    if (state == NULL_2_STATE && c =='l') {
        next_state = NULL_3_STATE;
    }
    if (state == NULL_3_STATE && c =='l') {
        next_state = WHITESPACE_STATE;
        is_completed = true;
        latest_token = make_pair(NULL_TOKEN, "null");
    }  
}

void handle_string_state(char &c, int &char_code) {
    int unicode_multiplier = 1;
    if (state == UNICODE_1_STATE) {
        unicode_multiplier = 16*16*16;
    }
    if (state == UNICODE_2_STATE) {
        unicode_multiplier = 16*16;
    }
    if (state == UNICODE_3_STATE) {
        unicode_multiplier = 16;
    }
    if (state == UNICODE_4_STATE) {
        unicode_multiplier = 1;
    }
    if (state == STRING_ESCAPE_STATE) {

        next_state =  STRING_STATE;
        vector<char> char_list_1 = {'b', 'f', 'n', 't', 'r', '/', '\\','\"'};
        vector<string> char_list_2 = {"\b", "\f", "\n", "\t", "\r", "/", "\\","\""};
        bool matched = false;
        for (int ch = 0; ch< char_list_1.size(); ch++) {
            if (char_list_1[ch] == c) {
                tokens.push_back(char_list_2[ch]);
                add_char = false;
            }
        }
        if (c == 'u' && !matched) {
            next_state = UNICODE_1_STATE;
            char_code = 0;
        }
    }
    else if (state == STRING_STATE) {
        if (c == '\"') {
            is_completed = true;
            latest_token = make_pair(STRING_TOKEN, mergeTokens());
            next_state = STRING_END_STATE;
            // cout<<"Debug 1:"<< mergeTokens()<<endl;
        }
        else if (c == '\\') {
            next_state = STRING_ESCAPE_STATE;
        }
        else {
            add_char = true;
        }
    }
    else if (state == STRING_END_STATE) {
        if (is_delimiter(c)) {
            should_advance = false;
            next_state = WHITESPACE_STATE;
        }

    }

    else if (state == UNICODE_1_STATE || state == UNICODE_2_STATE || state == UNICODE_3_STATE || state == UNICODE_4_STATE){
        if (char_range_v1(c, '0', '9')) {
            char_code += (c - 48) * unicode_multiplier; 
        }
        else if (char_range_v1(c, 'a', 'f')) {
            char_code += (c - 87) * unicode_multiplier; 
        }
        else if (char_range_v1(c, 'A', 'F')) {
            char_code += (c - 55) * unicode_multiplier; 
        }

        if (state == UNICODE_1_STATE) next_state = UNICODE_2_STATE;
        if (state == UNICODE_2_STATE) next_state = UNICODE_3_STATE;
        if (state == UNICODE_3_STATE) next_state = UNICODE_4_STATE;
        if (state == UNICODE_4_STATE) {
            next_state = STRING_STATE;
            c = char_code;
            add_char = true;
        }
    }

    
}

void processSingleCharacter(char c, int char_code) {

    // cout<<"Code debug 2: "<<c<<" "<<char_code<<" "<<state<<endl;
    should_advance = true;
    add_char = false;
    next_state = state;

    {
        if (state == WHITESPACE_STATE) { handle_whitespace_state(c,char_code); }
        else if (state == INTEGER_0_STATE || state == INTEGER_SIGN_STATE || state == INTEGER_STATE || state == INTEGER_EXP_STATE || state == INTEGER_EXP_0_STATE) { handle_integer_state(c,char_code); }
        else if (state == FLOATING_POINT_0_STATE || state == FLOATING_POINT_STATE) { handle_float_state(c,char_code); }
        else if (state == TRUE_1_STATE || state == TRUE_2_STATE || state == TRUE_3_STATE) { handle_true_state(c,char_code); }
        else if (state == FALSE_1_STATE || state == FALSE_2_STATE || state == FALSE_3_STATE || state == FALSE_4_STATE) { handle_false_state(c,char_code); }
        else if (state == NULL_1_STATE || state == NULL_2_STATE || state == NULL_3_STATE) { handle_null_state(c,char_code); }
        else { handle_string_state(c,char_code); }
    }
    if (add_char) {
        string s="";
        s.push_back(c);
        tokens.push_back(s);
    }
}

void start_tokenize_v1(char &c) {
    FState uu= TRUE_2_STATE;
    // cout<<"Test 3.0: "<< should_advance<< " "<< state << " "<<uu<< " "<<char_code<<endl;

    processSingleCharacter(c, char_code);
    state = next_state;
    // cout<<"Test 3.1: "<< should_advance<< " "<< state << " "<<uu<< " "<<char_code;
    if (is_completed) {
        is_completed = false;
        tokens.clear();
        // cout<<"Test 4: "<< should_advance<< state<< char_code;
        process_new_token(latest_token);
    }

}

Dispatcher<JsonEventType> getEventDispatcher() {
    return eventDispatcher;
}

void startJsonStreaming(string fileName) {
    tokens.clear();
    is_completed = false;
    char_code = 0;
    latest_token = make_pair(NULL_TOKEN,"");
    state = WHITESPACE_STATE;


    std::fstream fs{ fileName }; 
    fs >> std::noskipws;
    char c;
    int idx = 0;
    while (true && !stop_emitting_event) 
    { 
        if (should_advance) {
            if (fs >> c) {
                start_tokenize_v1(c);
            }
            else {
                break;
            }  
        }
        else {
            // std::cout << c << endl;
            start_tokenize_v1(c);
        }
         
    }
    JsonStreamEvent jsonStreamEvent(topicName, StreamToken(JsonEventType::Document_END, JsonEventType::Document_END, ""));
    eventDispatcherV1.post(jsonStreamEvent);
}
};
