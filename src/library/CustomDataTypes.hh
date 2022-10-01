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

class JsonPathKey {
public:
    bool isStringKey = false, anyIndex = false, anyKey = false;
    int index;
    string key;

    JsonPathKey() = default;

    explicit JsonPathKey(int index) {
        this->index = index;
        anyIndex = (index == -2);
    };

    JsonPathKey(string key, bool dummy) {
        this->key = key;
        isStringKey = true;
        anyKey = (key.length() == 0 || key == "*");
    };
};