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
    bool isDefault = true;
    bool isStringValue = false;
    string value;

    StreamToken(JsonTokenType _token_type, string _value, bool _isStringValue = false) : tokenType(_token_type),
                                                                                         value(_value),
                                                                                         isDefault(false),
                                                                                         isStringValue(
                                                                                                 _isStringValue) {};

    StreamToken() = default;

};

class Node {
public:
    bool isKey = false, anyIndex = false, anyKey = false, zeroOrMoreKey = false;
    int index;
    string key;

    Node() = default;

    explicit Node(int index) {
        this->index = index;
        anyIndex = (index == -2);
    };

    Node(string key, bool dummy) {
        this->key = key;
        isKey = true;
        anyKey = (key.length() == 0 || key == "*");
        zeroOrMoreKey = (key.length() == 0);
    };

    // void printNode() {
    //     cout<<"isKey: "<<isKey<<"key: "<<key<<"index: "<<index<<endl;
    // }

    bool nodeMatched(Node node) {
        return (node.isKey && (anyKey || node.key==key)) ||
            (!node.isKey && (anyIndex || node.index == index));
    }
};

class DFAState {
public:
    bool isDefault = true;
    string jsonPath;
    set<int> automationCurrentStates;
    bool indexesInCurrentJsonPathStackWhenDfaReachedAcceptStatesCanNotBeCleared = false;
    set<int> lengthOfCurrentJsonPathStackWhenDfaReachedAcceptState;

    DFAState() { init(); }

    DFAState(string jsonPath) {
        init();
        this->jsonPath = jsonPath;
        isDefault = false;
    }

    void init() {
        jsonPath = "$";
        automationCurrentStates.clear();
        lengthOfCurrentJsonPathStackWhenDfaReachedAcceptState.clear();
    }

    void clearVariablesWhichCanBeDeleted() {
        jsonPath.clear();
        automationCurrentStates.clear();
        if (!indexesInCurrentJsonPathStackWhenDfaReachedAcceptStatesCanNotBeCleared) {
            lengthOfCurrentJsonPathStackWhenDfaReachedAcceptState.clear();
        }
    }

    void updateCurrentAutomationStates(int state, int acceptState, int currentProcessingIndexInJsonPathStack) {
        if (state == acceptState) lengthOfCurrentJsonPathStackWhenDfaReachedAcceptState.insert(currentProcessingIndexInJsonPathStack);
        else automationCurrentStates.insert(state);
    }
};