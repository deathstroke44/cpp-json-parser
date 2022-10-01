#include <bits/stdc++.h>
#include "Dispatcher.hh"
#include "Event.hh"

using namespace std;


class JsonStreamParser {
    vector<string> tokens;
    bool isCompleted = false;
    int charCode = 0;
    StateEnum state = WHITESPACE_STATE;
    bool shouldAdvance = true;
    bool addChar = false;
    StateEnum nextState = WHITESPACE_STATE;
    pair<TokenType, string> previousToken;
    bool previousTokenUnProcessed = false;
    pair<TokenType, string> latestToken = make_pair(NULL_TOKEN, "");

public:
    Dispatcher<string> eventDispatcher = Dispatcher<string>();
    bool stopEmittingEvent = false;
    string topicName = "jsonStreamTopic";

    void setEventHandler(void (*func)(const JsonStreamEvent<string> &)) {
        eventDispatcher.subscribe(topicName, func);
    }

    void emitEvent(JsonTokenType jsonEventType, string value, bool isStringValue = false) {
        JsonStreamEvent jsonStreamEvent(topicName, StreamToken(jsonEventType, value, isStringValue));
        eventDispatcher.post(jsonStreamEvent);
    }

    void processPreviousToken(pair<TokenType, string> previousToken, pair<TokenType, string> currentToken) {
        currentToken.second == ":" ? emitEvent(KEY_TOKEN, previousToken.second)
                                   : emitEvent(VALUE_TOKEN, previousToken.second, true);
    }

    void streamToken(pair<TokenType, string> token) {
        if (previousTokenUnProcessed) {
            processPreviousToken(previousToken, token);
        }
        previousTokenUnProcessed = false;
        TokenType tokenType = token.first;
        string tokenValue = token.second;
        previousToken = token;
        if (tokenType == OPERATOR_TOKEN) {
            if (tokenValue == "[") {
                emitEvent(LIST_STARTED_TOKEN, "list");
            } else if (tokenValue == "]") {
                emitEvent(LIST_ENDED_TOKEN, "list");
            } else if (tokenValue == "}") {
                emitEvent(OBJECT_ENDED_TOKEN, "object");
            } else if (tokenValue == "{") {
                emitEvent(OBJECT_STARTED_TOKEN, "object");
            }
        } else if (tokenType == STRING_TOKEN) {
            previousTokenUnProcessed = true;
            return;
        } else if (tokenType == INTEGER_TOKEN || tokenType == FLOAT_TOKEN || tokenType == EXP_TOKEN
                   || tokenType == BOOLEAN_TOKEN || tokenType == NULL_TOKEN) {
            emitEvent(VALUE_TOKEN, tokenValue);
        }
    }

    void processNewToken(pair<TokenType, string> token) {
        streamToken(token);
    }

    string mergeTokens() {
        string str;
        for (auto &token: tokens) {
            str.append(token);
        }
        return str;
    }

    static bool isDelimiter(char c) {
        return c == ' ' || c == '{' || c == '}' || c == '[' || c == ']' || c == ':' || c == ',';
    }

    static bool isCharInRange(char c, int _mn, int _mx) {
        return ((c - '0') >= _mn && (c - '0') <= _mx);
    }

    static bool isCharInRangeV1(char c, char _mn, char _mx) {
        return (c >= _mn && c <= _mx);
    }

    void handleWhitespaceState(char &c, int &char_code) {
        vector<char> common_cases = {'{', '}', '[', ']', ',', ':'};
        for (char &common_case: common_cases) {
            if (common_case == c) {
                string s;
                s.push_back(c);
                isCompleted = true;
                latestToken = make_pair(OPERATOR_TOKEN, s);
                return;
            }
        }
        switch (c) {
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
                if (isCharInRange(c, 1, 9)) {
                    nextState = INTEGER_STATE;
                    addChar = true;
                }
                break;
        }
    }

    void handleIntegerState(char &c, int &char_code) {
        switch (state) {
            case INTEGER_0_STATE: {
                if (c == '.') {
                    nextState = FLOATING_POINT_0_STATE;
                    addChar = true;
                } else if (c == 'e' || c == 'E') {
                    nextState = INTEGER_EXP_0_STATE;
                    addChar = true;
                } else if (isDelimiter(c)) {
                    nextState = WHITESPACE_STATE;
                    isCompleted = true;
                    latestToken = make_pair(INTEGER_TOKEN, "0");
                    shouldAdvance = false;
                }
                break;
            }

            case INTEGER_SIGN_STATE: {
                if (c == '0') {
                    nextState = INTEGER_0_STATE;
                    addChar = true;
                } else if (isCharInRange(c, 1, 9)) {
                    nextState = INTEGER_STATE;
                    addChar = true;
                }
                break;
            }

            case INTEGER_EXP_0_STATE: {
                if (c == '+' || c == '-' || isCharInRange(c, 0, 9)) {
                    nextState = INTEGER_EXP_STATE;
                    addChar = true;
                }
                break;
            }

            case INTEGER_EXP_STATE: {
                if (isCharInRange(c, 0, 9)) {
                    addChar = true;
                } else if (isDelimiter(c)) {
                    isCompleted = true;
                    latestToken = make_pair(EXP_TOKEN, mergeTokens());
                    nextState = WHITESPACE_STATE;
                    shouldAdvance = false;
                }
                break;
            }

            case INTEGER_STATE: {
                if (isCharInRange(c, 0, 9)) {
                    addChar = true;
                } else if (isDelimiter(c)) {
                    isCompleted = true;
                    latestToken = make_pair(INTEGER_TOKEN, mergeTokens());
                    nextState = WHITESPACE_STATE;
                    shouldAdvance = false;
                } else if (c == '.') {
                    addChar = true;
                    nextState = FLOATING_POINT_0_STATE;
                } else if (c == 'E' || c == 'e') {
                    addChar = true;
                    nextState = INTEGER_EXP_0_STATE;
                }
                break;
            }

            default:
                break;
        }
    }

    void handleFloatState(char &c, int &charCode) {
        if (state == FLOATING_POINT_STATE) {
            if (isCharInRange(c, 0, 9)) {
                addChar = true;
            } else if (c == 'e' or c == 'E') {
                nextState = INTEGER_EXP_0_STATE;
                addChar = true;
            } else if (isDelimiter(c)) {
                isCompleted = true;
                latestToken = make_pair(FLOAT_TOKEN, mergeTokens());
                nextState = WHITESPACE_STATE;
                shouldAdvance = false;
            }
        } else if (state == FLOATING_POINT_0_STATE) {
            if (isCharInRange(c, 0, 9)) {
                nextState = FLOATING_POINT_STATE;
                addChar = true;
            }
        }
    }

    void handleTrueState(char &c, int &charCode) {
        if (state == TRUE_1_STATE && c == 'r') {
            nextState = TRUE_2_STATE;
        }
        if (state == TRUE_2_STATE && c == 'u') {
            nextState = TRUE_3_STATE;
        }
        if (state == TRUE_3_STATE && c == 'e') {
            nextState = WHITESPACE_STATE;
            isCompleted = true;
            latestToken = make_pair(BOOLEAN_TOKEN, "true");
        }
    }

    void handleFalseState(char &c, int &charCode) {
        if (state == FALSE_1_STATE && c == 'a') {
            nextState = FALSE_2_STATE;
        } else if (state == FALSE_2_STATE && c == 'l') {
            nextState = FALSE_3_STATE;
        } else if (state == FALSE_3_STATE && c == 's') {
            nextState = FALSE_4_STATE;
        } else if (state == FALSE_4_STATE && c == 'e') {
            nextState = WHITESPACE_STATE;
            isCompleted = true;
            latestToken = make_pair(BOOLEAN_TOKEN, "false");
        }
    }

    void handleNullState(char &c, int &charCode) {
        if (state == NULL_1_STATE && c == 'u') {
            nextState = NULL_2_STATE;
        }
        if (state == NULL_2_STATE && c == 'l') {
            nextState = NULL_3_STATE;
        }
        if (state == NULL_3_STATE && c == 'l') {
            nextState = WHITESPACE_STATE;
            isCompleted = true;
            latestToken = make_pair(NULL_TOKEN, "null");
        }
    }

    void handleStringState(char &c, int &charCode) {
        int unicodeMultiplier = 1;
        if (state == UNICODE_1_STATE) {
            unicodeMultiplier = 16 * 16 * 16;
        }
        if (state == UNICODE_2_STATE) {
            unicodeMultiplier = 16 * 16;
        }
        if (state == UNICODE_3_STATE) {
            unicodeMultiplier = 16;
        }
        if (state == UNICODE_4_STATE) {
            unicodeMultiplier = 1;
        }
        if (state == STRING_ESCAPE_STATE) {

            nextState = STRING_STATE;
            vector<char> charList1 = {'b', 'f', 'n', 't', 'r', '/', '\\', '\"'};
            vector<string> charList2 = {"\b", "\f", "\n", "\t", "\r", "/", "\\", "\""};
            bool matched = false;
            for (int ch = 0; ch < charList1.size(); ch++) {
                if (charList1[ch] == c) {
                    tokens.push_back(charList2[ch]);
                    addChar = false;
                    matched = true;
                }
            }
            if (c == 'u' && !matched) {
                nextState = UNICODE_1_STATE;
                charCode = 0;
            }
        } else if (state == STRING_STATE) {
            if (c == '\"') {
                isCompleted = true;
                latestToken = make_pair(STRING_TOKEN, mergeTokens());
                nextState = STRING_END_STATE;
            } else if (c == '\\') {
                nextState = STRING_ESCAPE_STATE;
            } else {
                addChar = true;
            }
        } else if (state == STRING_END_STATE) {
            if (isDelimiter(c)) {
                shouldAdvance = false;
                nextState = WHITESPACE_STATE;
            }
        } else if (state == UNICODE_1_STATE || state == UNICODE_2_STATE || state == UNICODE_3_STATE ||
                   state == UNICODE_4_STATE) {
            if (isCharInRangeV1(c, '0', '9')) {
                charCode += (c - 48) * unicodeMultiplier;
            } else if (isCharInRangeV1(c, 'a', 'f')) {
                charCode += (c - 87) * unicodeMultiplier;
            } else if (isCharInRangeV1(c, 'A', 'F')) {
                charCode += (c - 55) * unicodeMultiplier;
            }

            if (state == UNICODE_1_STATE)
                nextState = UNICODE_2_STATE;
            if (state == UNICODE_2_STATE)
                nextState = UNICODE_3_STATE;
            if (state == UNICODE_3_STATE)
                nextState = UNICODE_4_STATE;
            if (state == UNICODE_4_STATE) {
                nextState = STRING_STATE;
                c = charCode;
                addChar = true;
            }
        }
    }

    void processSingleCharacter(char c, int charCode) {
        shouldAdvance = true;
        addChar = false;
        nextState = state;

        if (state == WHITESPACE_STATE) {
            handleWhitespaceState(c, charCode);
        } else if (state == INTEGER_0_STATE || state == INTEGER_SIGN_STATE || state == INTEGER_STATE ||
                   state == INTEGER_EXP_STATE || state == INTEGER_EXP_0_STATE) {
            handleIntegerState(c, charCode);
        } else if (state == FLOATING_POINT_0_STATE || state == FLOATING_POINT_STATE) {
            handleFloatState(c, charCode);
        } else if (state == TRUE_1_STATE || state == TRUE_2_STATE || state == TRUE_3_STATE) {
            handleTrueState(c, charCode);
        } else if (state == FALSE_1_STATE || state == FALSE_2_STATE || state == FALSE_3_STATE ||
                   state == FALSE_4_STATE) {
            handleFalseState(c, charCode);
        } else if (state == NULL_1_STATE || state == NULL_2_STATE || state == NULL_3_STATE) {
            handleNullState(c, charCode);
        } else {
            handleStringState(c, charCode);
        }

        if (addChar) {
            string s;
            s.push_back(c);
            tokens.push_back(s);
        }
    }

    void startTokenize(char &c) {
        processSingleCharacter(c, charCode);
        state = nextState;
        if (isCompleted) {
            isCompleted = false;
            tokens.clear();
            processNewToken(latestToken);
        }
    }

    void startJsonStreaming(const string &fileName) {
        tokens.clear();
        isCompleted = false;
        charCode = 0;
        latestToken = make_pair(NULL_TOKEN, "");
        state = WHITESPACE_STATE;

        std::fstream fs{fileName};
        fs >> std::noskipws;
        char c;
        while (!stopEmittingEvent) {
            if (shouldAdvance) {
                if (fs >> c) {
                    startTokenize(c);
                } else {
                    break;
                }
            } else {
                startTokenize(c);
            }
        }
        JsonStreamEvent jsonStreamEvent(topicName, StreamToken(JsonTokenType::DOCUMENT_END_TOKEN, ""));
        eventDispatcher.post(jsonStreamEvent);
    }
};
