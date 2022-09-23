#include <bits/stdc++.h>
#include "Dispatcher.hh"
#include "Event.hh"
using namespace std;
typedef long long int li;


enum FState
{
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
class JsonStreamParser
{

    vector<string> tokens;
    bool nextStringWillBeKey = false;
    bool isCompleted = false;
    int charCode = 0;
    FState state = WHITESPACE_STATE;
    bool shouldAdvance = true;
    bool addChar = false;
    FState nextState = WHITESPACE_STATE;
    pair<TokenType, string> previous_token;
    bool previousTokenUnProcessed = false;
    pair<TokenType, string> latestToken = make_pair(NULL_TOKEN, "");

public:
    Dispatcher<string> eventDispatcher = Dispatcher<string>();
    bool stop_emitting_event = false;
    string topicName = "jsonStreamTopic";

    void setEventHandler(void (*func)(const JsonStreamEvent<string> &))
    {
        eventDispatcher.subscribe(topicName, func);
    }

    void emitEvent(int eventType, string value, bool listElement)
    {
        JsonEventType _JsonSubEventType = JsonEventType::VALUE_TOKEN;
        JsonEventType _JsonEventType;
        bool isStringValue = false;
        string _value = "";
        if (eventType == 0)
        {
            _JsonEventType = JsonEventType::VALUE_TOKEN;
            isStringValue = true;
            _value = value;
        }
        else if (eventType == 2)
        {
            _JsonEventType = KEY_TOKEN;
            _value = value;
        }
        else if (eventType == 3)
        {
            _JsonEventType = LIST_STARTED_TOKEN;
            _value = "list started";
        }
        else if (eventType == 4)
        {
            _JsonEventType = LIST_ENDED_TOKEN;
            _value = "list ended";
        }
        else if (eventType == 6)
        {
            _JsonEventType = OBJECT_STARTED_TOKEN;
            _value = "object started";
        }
        else if (eventType == 7)
        {
            _JsonEventType = OBJECT_ENDED_TOKEN;
            _value = "object ended";
        }
        else if (eventType == 8)
        {
            _JsonEventType = JsonEventType::VALUE_TOKEN;
            _value = value;
        }
        else if (eventType == 9)
        {
            _JsonEventType = VALUE_TOKEN;
            _value = value;
        }
        else if (eventType == 10)
        {
            _JsonEventType = VALUE_TOKEN;
            _value = value;
        }
        else if (eventType == 11)
        {
            _JsonEventType = VALUE_TOKEN;
            _value = value;
        }
        else if (eventType == 12)
        {
            _JsonEventType = VALUE_TOKEN;
            _value = value;
        }
        if (eventType == 0 || (eventType >= 2 && eventType <= 12))
        {
            JsonStreamEvent jsonStreamEvent(topicName, StreamToken(_JsonEventType, _JsonSubEventType, _value, isStringValue));
            eventDispatcher.post(jsonStreamEvent);
        }
    }

    void process_prev_token(pair<TokenType, string> prev_token, pair<TokenType, string> curr_token)
    {
        if (curr_token.second == ":")
        {
            emitEvent(2, prev_token.second, false);
        }
        else
        {
            emitEvent(0, prev_token.second, false);
        }
    }

    void stream_token(pair<TokenType, string> token, bool print_key_end_event)
    {
        if (previousTokenUnProcessed)
        {
            process_prev_token(previous_token, token);
        }
        previousTokenUnProcessed = false;
        TokenType token_type = token.first;
        string token_value = token.second;
        previous_token = token;
        if (token_type == OPERATOR_TOKEN)
        {
            if (token_value == "[")
            {
                emitEvent(3, "list", false);
            }
            else if (token_value == "]")
            {
                emitEvent(4, "list", false);
            }
            else if (token_value == "}")
            {
                emitEvent(7, "object", false);
            }
            else if (token_value == "{")
            {
                emitEvent(6, "object", false);
            }
        }
        else if (token_type == STRING_TOKEN)
        {
            previousTokenUnProcessed = true;
            return;
        }
        else if (token_type == INTEGER_TOKEN)
        {
            emitEvent(8, token_value, false);
        }
        else if (token_type == FLOAT_TOKEN)
        {
            emitEvent(9, token_value, false);
        }
        else if (token_type == EXP_TOKEN)
        {
            emitEvent(11, token_value, false);
        }
        else if (token_type == BOOLEAN_TOKEN)
        {
            emitEvent(10, token_value, false);
        }
        else if (token_type == NULL_TOKEN)
        {
            emitEvent(12, token_value, false);
        }
    }

    void processNewToken(pair<TokenType, string> token)
    {
        // cout << "\nNew Generated Token: "<< token.first << " "<<token.second<<endl;
        stream_token(token, true);
    }

    string mergeTokens()
    {
        string str = "";
        for (int i = 0; i < tokens.size(); i++)
        {
            // cout<<"\nTokens "<< tokens[i]<< endl;
            str.append(tokens.at(i));
        }
        return str;
    }

    bool is_delimiter(char c)
    {
        return c == ' ' || c == '{' || c == '}' || c == '[' || c == ']' || c == ':' || c == ',';
    }

    bool char_range(char c, int _mn, int _mx)
    {
        return ((c - '0') >= _mn && (c - '0') <= _mx);
    }

    bool char_range_v1(char c, char _mn, char _mx)
    {
        return (c >= _mn && c <= _mx);
    }

    void handleWhitespaceState(char &c, int &char_code)
    {
        vector<char> common_cases = {'{', '}', '[', ']', ',', ':'};
        for (auto i = common_cases.begin(); i != common_cases.end(); ++i)
        {
            if (*i == c)
            {
                string s;
                s.push_back(c);
                isCompleted = true;
                latestToken = make_pair(OPERATOR_TOKEN, s);
                return;
            }
        }
        switch (c)
        {
        case '\"':
            /* code */
            nextState = STRING_STATE;
            break;
        case '0':
            /* code */
            nextState = INTEGER_0_STATE;
            addChar = true;
            break;
        case '-':
            /* code */
            nextState = INTEGER_SIGN_STATE;
            addChar = true;
            break;
        case 'f':
            /* code */
            nextState = FALSE_1_STATE;
            break;
        case 't':
            /* code */
            nextState = TRUE_1_STATE;
            break;
        case 'n':
            /* code */
            nextState = NULL_1_STATE;
            break;

        default:
            if (char_range(c, 1, 9))
            {
                nextState = INTEGER_STATE;
                addChar = true;
            }
            break;
        }
    }

    void handleIntegerState(char &c, int &char_code)
    {
        switch (state)
        {
        case INTEGER_0_STATE:
        {
            if (c == '.')
            {
                nextState = FLOATING_POINT_0_STATE;
                addChar = true;
            }
            else if (c == 'e' || c == 'E')
            {
                nextState = INTEGER_EXP_0_STATE;
                addChar = true;
            }
            else if (is_delimiter(c))
            {
                nextState = WHITESPACE_STATE;
                isCompleted = true;
                latestToken = make_pair(INTEGER_TOKEN, "0");
                shouldAdvance = false;
            }
            break;
        }

        case INTEGER_SIGN_STATE:
        {
            if (c == '0')
            {
                nextState = INTEGER_0_STATE;
                addChar = true;
            }
            else if (char_range(c, 1, 9))
            {
                nextState = INTEGER_STATE;
                addChar = true;
            }
            break;
        }

        case INTEGER_EXP_0_STATE:
        {
            if (c == '+' || c == '-' || char_range(c, 0, 9))
            {
                nextState = INTEGER_EXP_STATE;
                addChar = true;
            }
            break;
        }

        case INTEGER_EXP_STATE:
        {
            if (char_range(c, 0, 9))
            {
                addChar = true;
            }
            else if (is_delimiter(c))
            {
                isCompleted = true;
                latestToken = make_pair(EXP_TOKEN, mergeTokens());
                nextState = WHITESPACE_STATE;
                shouldAdvance = false;
            }
            break;
        }

        case INTEGER_STATE:
        {
            if (char_range(c, 0, 9))
            {
                addChar = true;
            }
            else if (is_delimiter(c))
            {
                isCompleted = true;
                latestToken = make_pair(INTEGER_TOKEN, mergeTokens());
                nextState = WHITESPACE_STATE;
                shouldAdvance = false;
            }
            else if (c == '.')
            {
                addChar = true;
                nextState = FLOATING_POINT_0_STATE;
            }
            else if (c == 'E' || c == 'e')
            {
                addChar = true;
                nextState = INTEGER_EXP_0_STATE;
            }
            break;
        }

        default:
            break;
        }
    }

    void handleFloatState(char &c, int &char_code)
    {
        if (state == FLOATING_POINT_STATE)
        {
            if (char_range(c, 0, 9))
            {
                addChar = true;
            }
            else if (c == 'e' or c == 'E')
            {
                nextState = INTEGER_EXP_0_STATE;
                addChar = true;
            }

            else if (is_delimiter(c))
            {
                isCompleted = true;
                latestToken = make_pair(FLOAT_TOKEN, mergeTokens());
                nextState = WHITESPACE_STATE;
                shouldAdvance = false;
            }
        }
        else if (state == FLOATING_POINT_0_STATE)
        {
            if (char_range(c, 0, 9))
            {
                nextState = FLOATING_POINT_STATE;
                addChar = true;
            }
        }
    }

    void handleTrueState(char &c, int &char_code)
    {
        if (state == TRUE_1_STATE && c == 'r')
        {
            nextState = TRUE_2_STATE;
        }
        if (state == TRUE_2_STATE && c == 'u')
        {
            nextState = TRUE_3_STATE;
        }
        if (state == TRUE_3_STATE && c == 'e')
        {
            nextState = WHITESPACE_STATE;
            isCompleted = true;
            latestToken = make_pair(BOOLEAN_TOKEN, "true");
        }
    }

    void handleFalseState(char &c, int &char_code)
    {
        if (state == FALSE_1_STATE && c == 'a')
        {
            nextState = FALSE_2_STATE;
        }
        else if (state == FALSE_2_STATE && c == 'l')
        {
            nextState = FALSE_3_STATE;
        }
        else if (state == FALSE_3_STATE && c == 's')
        {
            nextState = FALSE_4_STATE;
        }
        else if (state == FALSE_4_STATE && c == 'e')
        {
            nextState = WHITESPACE_STATE;
            isCompleted = true;
            latestToken = make_pair(BOOLEAN_TOKEN, "false");
        }
    }

    void handleNullState(char &c, int &char_code)
    {
        if (state == NULL_1_STATE && c == 'u')
        {
            nextState = NULL_2_STATE;
        }
        if (state == NULL_2_STATE && c == 'l')
        {
            nextState = NULL_3_STATE;
        }
        if (state == NULL_3_STATE && c == 'l')
        {
            nextState = WHITESPACE_STATE;
            isCompleted = true;
            latestToken = make_pair(NULL_TOKEN, "null");
        }
    }

    void handleStringState(char &c, int &char_code)
    {
        int unicode_multiplier = 1;
        if (state == UNICODE_1_STATE)
        {
            unicode_multiplier = 16 * 16 * 16;
        }
        if (state == UNICODE_2_STATE)
        {
            unicode_multiplier = 16 * 16;
        }
        if (state == UNICODE_3_STATE)
        {
            unicode_multiplier = 16;
        }
        if (state == UNICODE_4_STATE)
        {
            unicode_multiplier = 1;
        }
        if (state == STRING_ESCAPE_STATE)
        {

            nextState = STRING_STATE;
            vector<char> char_list_1 = {'b', 'f', 'n', 't', 'r', '/', '\\', '\"'};
            vector<string> char_list_2 = {"\b", "\f", "\n", "\t", "\r", "/", "\\", "\""};
            bool matched = false;
            for (int ch = 0; ch < char_list_1.size(); ch++)
            {
                if (char_list_1[ch] == c)
                {
                    tokens.push_back(char_list_2[ch]);
                    addChar = false;
                }
            }
            if (c == 'u' && !matched)
            {
                nextState = UNICODE_1_STATE;
                char_code = 0;
            }
        }
        else if (state == STRING_STATE)
        {
            if (c == '\"')
            {
                isCompleted = true;
                latestToken = make_pair(STRING_TOKEN, mergeTokens());
                nextState = STRING_END_STATE;
                // cout<<"Debug 1:"<< mergeTokens()<<endl;
            }
            else if (c == '\\')
            {
                nextState = STRING_ESCAPE_STATE;
            }
            else
            {
                addChar = true;
            }
        }
        else if (state == STRING_END_STATE)
        {
            if (is_delimiter(c))
            {
                shouldAdvance = false;
                nextState = WHITESPACE_STATE;
            }
        }

        else if (state == UNICODE_1_STATE || state == UNICODE_2_STATE || state == UNICODE_3_STATE || state == UNICODE_4_STATE)
        {
            if (char_range_v1(c, '0', '9'))
            {
                char_code += (c - 48) * unicode_multiplier;
            }
            else if (char_range_v1(c, 'a', 'f'))
            {
                char_code += (c - 87) * unicode_multiplier;
            }
            else if (char_range_v1(c, 'A', 'F'))
            {
                char_code += (c - 55) * unicode_multiplier;
            }

            if (state == UNICODE_1_STATE)
                nextState = UNICODE_2_STATE;
            if (state == UNICODE_2_STATE)
                nextState = UNICODE_3_STATE;
            if (state == UNICODE_3_STATE)
                nextState = UNICODE_4_STATE;
            if (state == UNICODE_4_STATE)
            {
                nextState = STRING_STATE;
                c = char_code;
                addChar = true;
            }
        }
    }

    void processSingleCharacter(char c, int charCode)
    {
        shouldAdvance = true;
        addChar = false;
        nextState = state;

        {
            if (state == WHITESPACE_STATE)
            {
                handleWhitespaceState(c, charCode);
            }
            else if (state == INTEGER_0_STATE || state == INTEGER_SIGN_STATE || state == INTEGER_STATE || state == INTEGER_EXP_STATE || state == INTEGER_EXP_0_STATE)
            {
                handleIntegerState(c, charCode);
            }
            else if (state == FLOATING_POINT_0_STATE || state == FLOATING_POINT_STATE)
            {
                handleFloatState(c, charCode);
            }
            else if (state == TRUE_1_STATE || state == TRUE_2_STATE || state == TRUE_3_STATE)
            {
                handleTrueState(c, charCode);
            }
            else if (state == FALSE_1_STATE || state == FALSE_2_STATE || state == FALSE_3_STATE || state == FALSE_4_STATE)
            {
                handleFalseState(c, charCode);
            }
            else if (state == NULL_1_STATE || state == NULL_2_STATE || state == NULL_3_STATE)
            {
                handleNullState(c, charCode);
            }
            else
            {
                handleStringState(c, charCode);
            }
        }
        if (addChar)
        {
            string s = "";
            s.push_back(c);
            tokens.push_back(s);
        }
    }

    void startTokenizeV1(char &c)
    {
        FState uu = TRUE_2_STATE;
        processSingleCharacter(c, charCode);
        state = nextState;
        if (isCompleted)
        {
            isCompleted = false;
            tokens.clear();
            processNewToken(latestToken);
        }
    }

    void startJsonStreaming(string fileName)
    {
        tokens.clear();
        isCompleted = false;
        charCode = 0;
        latestToken = make_pair(NULL_TOKEN, "");
        state = WHITESPACE_STATE;

        std::fstream fs{fileName};
        fs >> std::noskipws;
        char c;
        int idx = 0;
        while (true && !stop_emitting_event)
        {
            if (shouldAdvance)
            {
                if (fs >> c)
                {
                    startTokenizeV1(c);
                }
                else
                {
                    break;
                }
            }
            else
            {
                startTokenizeV1(c);
            }
        }
        JsonStreamEvent jsonStreamEvent(topicName, StreamToken(JsonEventType::DOCUMENT_END_TOKEN, JsonEventType::DOCUMENT_END_TOKEN, ""));
        eventDispatcher.post(jsonStreamEvent);
    }
};
