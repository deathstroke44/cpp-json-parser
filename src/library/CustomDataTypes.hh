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
  OPERATOR_TOKEN, STRING_TOKEN, EXP_TOKEN, FLOAT_TOKEN, INTEGER_TOKEN, BOOLEAN_TOKEN, NULL_TOKEN
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
	bool isNotCreatedByMe = true;
	bool isStringValue = false;
	string value;

	StreamToken(JsonTokenType _token_type, string _value, bool _isStringValue = false)
		: tokenType(_token_type), value(_value), isNotCreatedByMe(false), isStringValue(_isStringValue) {};

	StreamToken() = default;

};

class Node {
 public:
	bool isKeyNode = false;
	int indexValue;
	string keyValue;

	/**
	 * These two fields are used for representing processed query
	 */
	bool wildcard = false; // *
	bool recursiveDescent = false; // ..user


	/*
	 * ETF : Extended Transition Function
	 * outputOfETFExceptAcceptState = ETF(parentNode.outputOfETFExceptAcceptState,thisNode) - {ACCEPT-STATE}
	 * outputOfETFHasAcceptState = ETF(parentNode.outputOfETFExceptAcceptState,thisNode).has(ACCEPT-STATE)
	 */
	set<int> outputOfETFExceptAcceptState;
	bool outputOfETFHasAcceptState = false;

	Node() = default;

	Node(const string &value, bool isKeyNode) {
		this->isKeyNode = isKeyNode;
		if (isKeyNode) {
			this->keyValue = value;
			wildcard = (keyValue == "*");
			recursiveDescent = (keyValue.length() == 0);
		} else {
			if (value == "*") wildcard = true;
			else this->indexValue = stoi(value);
		}
	}

	explicit Node(int indexValue) {
		this->indexValue = indexValue;
	};

	[[nodiscard]] bool satisfyJsonPathQuery(const Node &node) const {
		return (node.isKeyNode == isKeyNode) && ((node.isKeyNode && (wildcard || node.keyValue == keyValue))
			|| (!node.isKeyNode && (wildcard || node.indexValue == indexValue)));
	}

	void clearAutomationStates() {
		outputOfETFExceptAcceptState.clear();
		outputOfETFHasAcceptState = false;
	}
};