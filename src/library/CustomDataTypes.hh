#include<bits/stdc++.h>

using namespace std;

enum StateEnum {
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

enum TokenType {
    OPERATOR_TOKEN,
    STRING_TOKEN,
    EXP_TOKEN,
    FLOAT_TOKEN,
    INTEGER_TOKEN,
    BOOLEAN_TOKEN,
    NULL_TOKEN
};

enum JsonTokenType {
    KEY_TOKEN,
    OBJECT_STARTED_TOKEN,
    OBJECT_ENDED_TOKEN,
    LIST_STARTED_TOKEN,
    LIST_ENDED_TOKEN,
    DOCUMENT_END_TOKEN,
    VALUE_TOKEN,
};

class StreamToken {
public:
    JsonTokenType tokenType;
    bool isCreatedByMe = true;
    bool isStringValue = false;
    string value;

    StreamToken(JsonTokenType _token_type, string _value, bool _isStringValue = false) : tokenType(_token_type),
                                                                                         value(_value),
                                                                                         isCreatedByMe(false),
                                                                                         isStringValue(
                                                                                                 _isStringValue) {};

    StreamToken() = default;

};

class Node {
public:
    bool isKeyNode = false;
    bool wildcard = false;
    bool recursiveDescent = false;
    int index;
    string key;

    set<int> automationStates;
    bool reachAcceptStates = false;

    Node() = default;
    
    Node(string value, bool isKeyNode) {
        this->isKeyNode = isKeyNode;
        if (isKeyNode) {
            this->key = value;
            wildcard = (key == "*");
            recursiveDescent = (key.length() == 0);
        }
        else {
            if (value=="*") wildcard = true;
            else this->index = stoi(value);
        }
    }

    explicit Node(int index) {
        this->index = index;
    };

    bool nodeMatched(Node node) {
        return (node.isKeyNode && (wildcard || recursiveDescent || node.key==key)) ||
            (!node.isKeyNode && (wildcard || node.index == index));
    }

    void updateCurrentAutomationStates(int state, int acceptState, int currentProcessingIndexInJsonPathStack) {
        if (state == acceptState) reachAcceptStates = true;
        else automationStates.insert(state);
    }
};

class DFAState {
public:
    bool isNotCreatedByMe = true;
    string jsonPath;
    set<int> automationCurrentStates;
    bool canClearLengthsOfCurrentJsonPathStackWhenDfaReachedAcceptState = false;
    set<int> lengthsOfCurrentJsonPathStackWhenDfaReachedAcceptState;

    DFAState() { init(); }

    DFAState(string jsonPath) {
        init();
        this->jsonPath = jsonPath;
        isNotCreatedByMe = false;
    }

    void init() {
        jsonPath = "$";
        automationCurrentStates.clear();
        lengthsOfCurrentJsonPathStackWhenDfaReachedAcceptState.clear();
    }

    void clearVariablesWhichCanBeDeleted() {
        jsonPath.clear();
        automationCurrentStates.clear();
        if (!canClearLengthsOfCurrentJsonPathStackWhenDfaReachedAcceptState) {
            lengthsOfCurrentJsonPathStackWhenDfaReachedAcceptState.clear();
        }
    }

    void updateCurrentAutomationStates(int state, int acceptState, int currentProcessingIndexInJsonPathStack) {
        if (state == acceptState) lengthsOfCurrentJsonPathStackWhenDfaReachedAcceptState.insert(currentProcessingIndexInJsonPathStack);
        else automationCurrentStates.insert(state);
    }

    bool valueOfJsonPathUptoIthIndexWillBeAddedInResult(int i) {
        return lengthsOfCurrentJsonPathStackWhenDfaReachedAcceptState.find(i) !=
                lengthsOfCurrentJsonPathStackWhenDfaReachedAcceptState.end();
    }
};