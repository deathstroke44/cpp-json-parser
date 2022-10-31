#include "library/JsonStreaming.hh"

using namespace std;

vector<Node> currentJsonPathList;
map<string, string> jsonPathQueryResultsMap;
map<string, StreamToken> lastAddedTokenInResultMap;
vector<Node> jsonPathQueryProcessed;
bool multipleResultExistForThisQuery = false;
int numberOfNodeInCurrentPathContainingAcceptStateInTheirETF = 0;
int acceptStateOfNfa;

bool delimiterNeededBeforeAppendingStreamTokenInResult(const string &jsonPath, const StreamToken &streamToken) {
	StreamToken lastAddedStreamTokenInThisJsonPath = lastAddedTokenInResultMap[jsonPath];
	if (lastAddedStreamTokenInThisJsonPath.isNotCreatedByMe) return false;
	JsonTokenType tokenTypeOfCurrentlyProcessingToken = streamToken.tokenType;
	if (tokenTypeOfCurrentlyProcessingToken == LIST_ENDED_TOKEN
		|| tokenTypeOfCurrentlyProcessingToken == OBJECT_ENDED_TOKEN)
		return false;
	if (tokenTypeOfCurrentlyProcessingToken == VALUE_TOKEN || tokenTypeOfCurrentlyProcessingToken == LIST_STARTED_TOKEN
		|| tokenTypeOfCurrentlyProcessingToken == OBJECT_STARTED_TOKEN) {
		return lastAddedStreamTokenInThisJsonPath.tokenType == VALUE_TOKEN
			|| lastAddedStreamTokenInThisJsonPath.tokenType == OBJECT_ENDED_TOKEN
			|| lastAddedStreamTokenInThisJsonPath.tokenType == LIST_ENDED_TOKEN;
	}
	if (tokenTypeOfCurrentlyProcessingToken == KEY_TOKEN)
		return lastAddedStreamTokenInThisJsonPath.tokenType != OBJECT_STARTED_TOKEN;
	return false;
}

void addStreamTokenInJsonPathQueryResult(const string &jsonPath, const StreamToken &streamToken) {
	string jsonPathQueryResultInThisJsonPath = jsonPathQueryResultsMap[jsonPath];
	jsonPathQueryResultInThisJsonPath.append(delimiterNeededBeforeAppendingStreamTokenInResult(jsonPath, streamToken)
											 ? "," : "");
	lastAddedTokenInResultMap[jsonPath] = streamToken;
	JsonTokenType jsonTokenType = streamToken.tokenType;
	if (jsonTokenType == JsonTokenType::KEY_TOKEN) {
		jsonPathQueryResultInThisJsonPath += "\"" + streamToken.value + "\"" + " : ";
	} else if (jsonTokenType == VALUE_TOKEN) {
		jsonPathQueryResultInThisJsonPath.append(streamToken.isStringValue ? "\"" + streamToken.value + "\""
																		   : streamToken.value);
	} else if (jsonTokenType == LIST_STARTED_TOKEN) {
		jsonPathQueryResultInThisJsonPath.push_back('[');
	} else if (jsonTokenType == LIST_ENDED_TOKEN) {
		jsonPathQueryResultInThisJsonPath.push_back(']');
	} else if (jsonTokenType == OBJECT_STARTED_TOKEN) {
		jsonPathQueryResultInThisJsonPath.push_back('{');
	} else if (jsonTokenType == OBJECT_ENDED_TOKEN) {
		jsonPathQueryResultInThisJsonPath.push_back('}');
	}
	jsonPathQueryResultsMap[jsonPath] = jsonPathQueryResultInThisJsonPath;
}

void updateResultIfJsonPathQuerySatisfies(const StreamToken &streamToken) {
	if (numberOfNodeInCurrentPathContainingAcceptStateInTheirETF) {
		string jsonPath;
		int numberOfJsonPathsAddedInResult = 0;
		for (const auto &node : currentJsonPathList) {
			if (numberOfJsonPathsAddedInResult >= numberOfNodeInCurrentPathContainingAcceptStateInTheirETF) return;
			if (node.isKeyNode) {
				jsonPath.append(node.keyValue != "$" ? "." + node.keyValue : node.keyValue);
			} else {
				jsonPath.append("[" + to_string(node.indexValue) + "]");
			}
			if (node.outputOfETFHasAcceptState) {
				numberOfJsonPathsAddedInResult++;
				addStreamTokenInJsonPathQueryResult(jsonPath, streamToken);
			}
		}
	}
}

void printJsonPathQueryResult() {
	string finalResult;
	for (const auto &result : jsonPathQueryResultsMap) {
		if (finalResult.length()) finalResult.push_back(',');
		finalResult.append(result.second);
	}
	if (multipleResultExistForThisQuery) finalResult = "[" + finalResult + "]";
	cout << "Got value of desired key final result: " << endl << finalResult << endl;
	for (const auto &result : jsonPathQueryResultsMap) {
		cout << "Result For this JSON path query: " << result.first << " -> " << result.second << endl;;
	}
}

vector<int> transitionFunction(int state, Node &childNode) {
	vector<int> result;
	if (state + 1 <= acceptStateOfNfa) {
		if (jsonPathQueryProcessed[state + 1].recursiveDescent) {
			result.emplace_back(state);
		}
		if (jsonPathQueryProcessed[state + 1].satisfyJsonPathQuery(childNode)) {
			result.emplace_back(state + 1);
		}
	}
	return result;
}

void extendedTransitionFunction(const set<int> &states, Node &childNode) {
	childNode.clearAutomationStates();
	for (auto state : states) {
		for (auto transitionedState : transitionFunction(state, childNode)) {
			if (transitionedState == acceptStateOfNfa) {
				childNode.outputOfETFHasAcceptState = true;
			} else {
				childNode.outputOfETFExceptAcceptState.insert(transitionedState);
			}
		}
	}
	if (childNode.outputOfETFHasAcceptState) {
		numberOfNodeInCurrentPathContainingAcceptStateInTheirETF++;
	}
}

Node getLastNodeOfCurrentJsonPath() {
	return currentJsonPathList.back();
}

Node getSecondLastNodeOfCurrentJsonPath() {
	return currentJsonPathList.at(currentJsonPathList.size() - 2);
}

bool isLastNodeOfCurrentJsonPathIsKeyNode() {
	return !currentJsonPathList.empty() && getLastNodeOfCurrentJsonPath().isKeyNode;
}

bool isLastNodeOfCurrentJsonPathIsIndexNode() {
	return !currentJsonPathList.empty() && !getLastNodeOfCurrentJsonPath().isKeyNode;
}

bool isIncrementIndexOfLastNodeIsNecessaryNowConsideringETFState() {
	return currentJsonPathList.size() - 2 >= 0
		&& !getSecondLastNodeOfCurrentJsonPath().outputOfETFExceptAcceptState.empty();
}

void popNodeFromCurrentJsonPath() {
	if (!currentJsonPathList.empty()) {
		Node &lastNodeOfCurrentJsonPath = currentJsonPathList.back();
		if (lastNodeOfCurrentJsonPath.outputOfETFHasAcceptState) {
			numberOfNodeInCurrentPathContainingAcceptStateInTheirETF--;
		}
		lastNodeOfCurrentJsonPath.clearAutomationStates();
		currentJsonPathList.pop_back();
	}
}

void removeLastNodeFromCurrentJsonPathIfItIsKeyNode() {
	if (isLastNodeOfCurrentJsonPathIsKeyNode()) {
		popNodeFromCurrentJsonPath();
	}
}

void pushKeyNodeInCurrentJsonPath(const string &key) {
	currentJsonPathList.emplace_back(key, true);
	extendedTransitionFunction(getSecondLastNodeOfCurrentJsonPath().outputOfETFExceptAcceptState,
							   currentJsonPathList.back());
}

void pushIndexNodeInCurrentJsonPath(int index) {
	currentJsonPathList.emplace_back(index);
}

void incrementIndexLastNodeOfCurrentJsonPath() {
	Node &lastNodeOfCurrentJsonPath = currentJsonPathList.back();
	if (lastNodeOfCurrentJsonPath.outputOfETFHasAcceptState) {
		numberOfNodeInCurrentPathContainingAcceptStateInTheirETF--;
	}
	lastNodeOfCurrentJsonPath.indexValue++;
	extendedTransitionFunction(getSecondLastNodeOfCurrentJsonPath().outputOfETFExceptAcceptState,
							   lastNodeOfCurrentJsonPath);
	if (getLastNodeOfCurrentJsonPath().outputOfETFHasAcceptState) {
		numberOfNodeInCurrentPathContainingAcceptStateInTheirETF++;
	}
}

void incrementIndexLastNodeOfCurrentJsonPathIfItIsIndexNode() {
	if (isLastNodeOfCurrentJsonPathIsIndexNode() && isIncrementIndexOfLastNodeIsNecessaryNowConsideringETFState()) {
		incrementIndexLastNodeOfCurrentJsonPath();
	}
}

void handleJsonStreamParserEvent(const JsonStreamEvent<string> &jsonStreamEvent) {
	StreamToken streamToken = jsonStreamEvent.getStreamToken();
	JsonTokenType streamTokenType = streamToken.tokenType;
	if (streamTokenType == KEY_TOKEN) {
		updateResultIfJsonPathQuerySatisfies(streamToken);
		pushKeyNodeInCurrentJsonPath(streamToken.value);
	} else if (streamTokenType == VALUE_TOKEN) {
		incrementIndexLastNodeOfCurrentJsonPathIfItIsIndexNode();
		updateResultIfJsonPathQuerySatisfies(streamToken);
		removeLastNodeFromCurrentJsonPathIfItIsKeyNode();
	} else if (streamTokenType == OBJECT_STARTED_TOKEN) {
		incrementIndexLastNodeOfCurrentJsonPathIfItIsIndexNode();
		updateResultIfJsonPathQuerySatisfies(streamToken);
	} else if (streamTokenType == LIST_STARTED_TOKEN) {
		incrementIndexLastNodeOfCurrentJsonPathIfItIsIndexNode();
		updateResultIfJsonPathQuerySatisfies(streamToken);
		pushIndexNodeInCurrentJsonPath(-1);
	} else if (streamTokenType == LIST_ENDED_TOKEN) {
		popNodeFromCurrentJsonPath();
		updateResultIfJsonPathQuerySatisfies(streamToken);
		removeLastNodeFromCurrentJsonPathIfItIsKeyNode();
	} else if (streamTokenType == OBJECT_ENDED_TOKEN) {
		updateResultIfJsonPathQuerySatisfies(streamToken);
		removeLastNodeFromCurrentJsonPathIfItIsKeyNode();
	} else if (streamTokenType == DOCUMENT_END_TOKEN) {
		printJsonPathQueryResult();
	}
}

void processJsonPathQuery(const string &jsonPathQuery) {
	vector<Node> jsonPathQueryTokenized;
	size_t indexOfNextDot = jsonPathQuery.find('.');
	size_t indexOfNextLeftSquareBracket;
	size_t indexOfNextRightSquareBracket;
	vector<int> occurrenceIndexesOfDotInQuery;
	jsonPathQueryTokenized.emplace_back("$", true);
	while (indexOfNextDot != string::npos) {
		occurrenceIndexesOfDotInQuery.emplace_back(indexOfNextDot);
		indexOfNextDot = jsonPathQuery.find('.', indexOfNextDot + 1);
	}
	for (int i = 0; i < occurrenceIndexesOfDotInQuery.size(); i++) {
		int ithOccurrenceOfDot = occurrenceIndexesOfDotInQuery[i] + 1;
		int iPlusOneOccurrenceOfDot = jsonPathQuery.size() - 1;
		if (i + 1 <= occurrenceIndexesOfDotInQuery.size() - 1) {
			iPlusOneOccurrenceOfDot = occurrenceIndexesOfDotInQuery[i + 1] - 1;
		}
		string str1;
		if (ithOccurrenceOfDot <= iPlusOneOccurrenceOfDot) {
			str1 = jsonPathQuery.substr(ithOccurrenceOfDot, iPlusOneOccurrenceOfDot - ithOccurrenceOfDot + 1);
		}
		indexOfNextLeftSquareBracket = str1.find('[');
		int indexOfStartOfFirstIndexNode = -1;
		while (indexOfNextLeftSquareBracket != string::npos) {
			indexOfNextRightSquareBracket = str1.find(']', indexOfNextLeftSquareBracket + 1);
			if (indexOfNextRightSquareBracket != string::npos) {
				if (indexOfStartOfFirstIndexNode == -1) {
					indexOfStartOfFirstIndexNode = indexOfNextLeftSquareBracket;
					jsonPathQueryTokenized.emplace_back(str1.substr(0, indexOfNextLeftSquareBracket), true);
				}
				jsonPathQueryTokenized.emplace_back(str1.substr(indexOfNextLeftSquareBracket + 1,
																(indexOfNextRightSquareBracket - 1)
																	- (indexOfNextLeftSquareBracket + 1) + 1), false);
				indexOfNextLeftSquareBracket = str1.find('[', indexOfNextRightSquareBracket + 1);
			} else break;
		}
		if (indexOfStartOfFirstIndexNode == -1) {
			jsonPathQueryTokenized.emplace_back(str1, true);
		}
	}
	for (auto &token : jsonPathQueryTokenized) {
		multipleResultExistForThisQuery = multipleResultExistForThisQuery || (token.recursiveDescent || token.wildcard);
	}
	for (int i = 0; i < jsonPathQueryTokenized.size(); i++) {
		Node &jsonQueryNode = jsonPathQueryTokenized.at(i);
		if (jsonPathQueryTokenized.at(i).recursiveDescent) {
			jsonPathQueryTokenized[i + 1].recursiveDescent = true;
			i++;
		}
		jsonPathQueryProcessed.emplace_back(jsonPathQueryTokenized.at(i));
	}
}

void initJsonPathQueryStates(string &jsonPathQuery) {
	Node node("$", true);
	if (jsonPathQuery == "$") {
		node.outputOfETFHasAcceptState = true;
	} else {
		node.outputOfETFExceptAcceptState.insert(0);
	}
	currentJsonPathList.emplace_back(node);
	processJsonPathQuery(jsonPathQuery);
	acceptStateOfNfa = jsonPathQueryProcessed.size() - 1;
}

void executeJsonPathQuery(string fileName, string jsonPathQuery) {
	cout << "JSON path query:" << jsonPathQuery << " -filename: " << fileName << endl;
	fileName = "tests/Json files/" + fileName;
	initJsonPathQueryStates(jsonPathQuery);
	JsonStreamParser jsonStreamParser = JsonStreamParser();
	jsonStreamParser.setEventHandler(handleJsonStreamParserEvent);
	jsonStreamParser.startJsonStreaming(fileName);
}

int main(int argc, char **argv) {
	freopen("output.txt", "a+", stdout);
	JsonStreamParser jsonStreamParser = JsonStreamParser();
	string fileName(argv[1]);
	string jsonPathQuery(argv[2]);
	executeJsonPathQuery(fileName, jsonPathQuery);
}