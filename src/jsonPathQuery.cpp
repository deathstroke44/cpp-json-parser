#include "library/JsonStreaming.hh"

/**
 * @brief 
 * ETF - Extended Transition Function 
 */

using namespace std;

vector<Node> currentJsonPathList;

/**
 * {
 * 	"user": {
 * 		"name": {
 * 			"name": {
 * 				"firstName": "Samin",
 * 				"LastName": "Yeaser"
 * 			}
 * 		}
 * 	}
 * }
 * Query: $.user..name for that json path query result will be [value($.user.name),value($.user.name.name)]
   = [{"name": {"firstName": "Samin", "LastName": "Yeaser"}}, {"firstName": "Samin", "LastName": "Yeaser"}]
 * So I need to store result for both these json path
 * So final result will be [jsonPathQueryResultsMap['$.user.name'],jsonPathQueryResultsMap['$.user.name.name']]
 * lastAddedTokenInResultMap['$.user.name'] will store last token added in jsonPathQueryResultsMap['$.user.name']
 * similarly lastAddedTokenInResultMap['$.user.name.name'] will store last token added in jsonPathQueryResultsMap['$.user.name.name']
 */
map<string, string> jsonPathQueryResultsMap;
map<string, StreamToken> lastAddedTokenInResultMap;

/**
 * Processed tokenized version of json path query
 * Each element of jsonPathQueryProcessed is a state of nfa and ith element represent
   condition of reaching (i-1)th state to (i-1)th state (if ith node is recursive descent) or ith state of nfa
 */
vector<Node> jsonPathQueryProcessed;
bool wildCardOrRecursiveDescentContainsInQuery = false;

/**
 * {
 * 	"user": {
 * 		"name": {
 * 			"name" : "omi".
 * 			"surName": "smn"
 * 		}
 * 	}
 * 	query = $.user..name
 *  if currentJsonPath is $.user.name.name then prefix paths are following [$.user.name.name ,$.user.name ,$.user, $]
 * 	after processing the more nested name node currentJsonPathList = ["$","user","name","name"]
 * 	Among them outputOfETFHasAcceptState = true in 3nd and 4th node in path
 * 	So in that case countOfPrefixPathsThatsETFOutputContainsAcceptState = 2
 * 	if countOfPrefixPathsThatsETFOutputContainsAcceptState > 0 means json path query satisfies
 * }
 */
int countOfPrefixPathsThatsETFOutputContainsAcceptState = 0;
/**
 * acceptStateOfNfa = jsonPathQueryProcessed.size()-1
 */
int acceptStateOfNfa;

/**
 *
 * @param jsonPath
 * @param streamToken
 * @return
     Example 1:
     current Token : KEY TOKEN
     current result Json : { 'name': 'Samin Yeaser'
     last added token: VALUE TOKEN
     So before adding current token to the JSON result I need to add a delimiter
     Example 2:
     current Token : KEY TOKEN
     current result Json : {
     last added token: VALUE TOKEN
     So before adding current token to the JSON result there is no need to add a delimiter
     This function determines if I need to add a delimiter or not
 */
bool needToAppendDelimiterInResultBeforeAppendingThisToken(const string &jsonPath, const StreamToken &streamToken) {
	StreamToken lastAddedStreamTokenInThisJsonPath = lastAddedTokenInResultMap[jsonPath];
	if (lastAddedStreamTokenInThisJsonPath.isNotCreatedByMe) return false;
	JsonTokenType tokenTypeOfCurrentlyProcessingToken = streamToken.tokenType;
	if (tokenTypeOfCurrentlyProcessingToken == LIST_ENDED_TOKEN
		|| tokenTypeOfCurrentlyProcessingToken == OBJECT_ENDED_TOKEN) {
		return false;
	}
	if (tokenTypeOfCurrentlyProcessingToken == VALUE_TOKEN || tokenTypeOfCurrentlyProcessingToken == LIST_STARTED_TOKEN
		|| tokenTypeOfCurrentlyProcessingToken == OBJECT_STARTED_TOKEN) {
		return (lastAddedStreamTokenInThisJsonPath.tokenType == VALUE_TOKEN
			|| lastAddedStreamTokenInThisJsonPath.tokenType == OBJECT_ENDED_TOKEN
			|| lastAddedStreamTokenInThisJsonPath.tokenType == LIST_ENDED_TOKEN);
	}
	if (tokenTypeOfCurrentlyProcessingToken == KEY_TOKEN) {
		return lastAddedStreamTokenInThisJsonPath.tokenType != OBJECT_STARTED_TOKEN;
	}
	return false;
}

/**
 * {
 * 	"user": {
 * 		"name": {
 * 			"name": {
 * 				"firstName": "Samin",
 * 				"LastName": "Yeaser"
 * 			}
 * 		}
 * 	}
 * }
 * Query: $.user..name for that path Output of result will be [value($.user.name),value($.user.name.name)]
   = [{"name": {"firstName": "Samin", "LastName": "Yeaser"}}, {"firstName": "Samin", "LastName": "Yeaser"}]
 * @param jsonPath : for this query and json value of that field can be $.user.details.name.name or $.user.details.name
 * @param streamToken "omi" or "name" or "{" or "}"
 */
void addStreamTokenInJsonPathQueryResult(const string &jsonPath, const StreamToken &streamToken) {
	string jsonPathQueryResultInThisJsonPath = jsonPathQueryResultsMap[jsonPath];
	if (needToAppendDelimiterInResultBeforeAppendingThisToken(jsonPath, streamToken)) {
		jsonPathQueryResultInThisJsonPath.append(",");
	}
	lastAddedTokenInResultMap[jsonPath] = streamToken;
	JsonTokenType jsonTokenType = streamToken.tokenType;

	if (jsonTokenType == JsonTokenType::KEY_TOKEN) {
		jsonPathQueryResultInThisJsonPath += "\"" + streamToken.value + "\"" + " : ";
	} else if (jsonTokenType == VALUE_TOKEN) {
		string streamTokenValue = streamToken.isStringValue ? "\"" + streamToken.value + "\"" : streamToken.value;
		jsonPathQueryResultInThisJsonPath.append(streamTokenValue);
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

bool isJsonPathQuerySatisfy() {
	return countOfPrefixPathsThatsETFOutputContainsAcceptState > 0;
}
/**
 * Update result with currently processing streamToken
 * @param streamToken
 */
void updateResult(const StreamToken &streamToken) {
	string jsonPath;
	/**
	 * This variable is added for optimization. So I won't have to iterate the whole list if not needed
	 */
	int numberOfJsonPathsAddedInResult = 0;
	for (const auto &node : currentJsonPathList) {
		if (numberOfJsonPathsAddedInResult >= countOfPrefixPathsThatsETFOutputContainsAcceptState) {
			return;
		}
		if (node.isKeyNode) {
			jsonPath.append(node.keyValue != "$" ? "." + node.keyValue : node.keyValue);
		} else {
			jsonPath.append("[" + to_string(node.indexValue) + "]");
		}
		if (node.outputOfETFUptoThisNodeInCurrentPathHasAcceptState) {
			numberOfJsonPathsAddedInResult++;
			addStreamTokenInJsonPathQueryResult(jsonPath, streamToken);
		}
	}
}

void printJsonPathQueryResult() {
	string finalResult;
	for (const auto &result : jsonPathQueryResultsMap) {
		if (finalResult.length()) finalResult.push_back(',');
		finalResult.append(result.second);
	}
	if (wildCardOrRecursiveDescentContainsInQuery) {
		finalResult = "[" + finalResult + "]";
	}
	cout << "Got value of desired key final result: " << endl << finalResult << endl;
	for (const auto &result : jsonPathQueryResultsMap) {
		cout << "Result For this JSON path query: " << result.first << " -> " << result.second << endl;;
	}
}

Node getLastNodeOfCurrentJsonPath() {
	return currentJsonPathList.back();
}

Node getSecondLastNodeOfCurrentJsonPath() {
	return currentJsonPathList.at(currentJsonPathList.size() - 2);
}

/**
 * @brief 
 * ETF - Extended Transition Function
 * currentJsonPathList = ["$","general","user","name"]
 * query: $..user.name
 * ETF(q0,{"$"}) = {0}
 * ETF(q0,{"$","general"}) = TF(0,"general") = {0}
 * ETF(q0,{"$","general","user"}) = TF(0,"..user") = {0,1}
 * ETF(q0,{"$","general","user","name"}) = TF(0,"name") U TF(1,"name") = {0} U {2} = {0,2}
 * We can see that ETF(q0,{"$","general","user","name"}) is using the output of ETF(q0,{"$","general","user"})
 * So we are storing output of ETF of a path ($.general.user) into the last node ("user" node) of the path.
 * So we can compute the ETF of currentJson path by using the ETF states of second last node in current json path
 * For example to compute ETF of $.general.user.name we ule use ETF of $.general.user which is stored in "user" node
 */
void computeExtendedTransitionFunctionForCurrentJsonPath() {
	Node &lastNodeOfCurrentJsonPath = currentJsonPathList.back();
	lastNodeOfCurrentJsonPath.clearAutomationStates();
	for (auto state : getSecondLastNodeOfCurrentJsonPath().outputOfETFUptoThisNodeInCurrentPathExceptAcceptState) {
		if (state + 1 <= acceptStateOfNfa) {
			if (jsonPathQueryProcessed[state + 1].recursiveDescent) {
				lastNodeOfCurrentJsonPath.outputOfETFUptoThisNodeInCurrentPathExceptAcceptState.insert(state);
			}
			if (jsonPathQueryProcessed[state + 1].satisfyJsonPathQuery(lastNodeOfCurrentJsonPath)) {
				if (state + 1 == acceptStateOfNfa) {
					lastNodeOfCurrentJsonPath.outputOfETFUptoThisNodeInCurrentPathHasAcceptState = true;
				} else {
					lastNodeOfCurrentJsonPath.outputOfETFUptoThisNodeInCurrentPathExceptAcceptState.insert(state + 1);
				}
			}
		}
	}
	if (lastNodeOfCurrentJsonPath.outputOfETFUptoThisNodeInCurrentPathHasAcceptState) {
		countOfPrefixPathsThatsETFOutputContainsAcceptState++;
	}
}

bool isLastNodeOfCurrentJsonPathIsKeyNode() {
	return !currentJsonPathList.empty() && getLastNodeOfCurrentJsonPath().isKeyNode;
}

bool isLastNodeOfCurrentJsonPathIsIndexNode() {
	return !currentJsonPathList.empty() && !getLastNodeOfCurrentJsonPath().isKeyNode;
}

/**
 * if parentNode of NewNode's outputOfETFExceptAcceptState then ETF(parentNode.outputOfETFExceptAcceptState, newNode) will be empty
  then incrementing the index of last node is completely unnecessary
 * @return
 */
bool isIncrementIndexOfLastNodeIsNecessaryNowConsideringETFState() {
	return currentJsonPathList.size() - 2 >= 0
		&& !getSecondLastNodeOfCurrentJsonPath().outputOfETFUptoThisNodeInCurrentPathExceptAcceptState.empty();
}

void popNodeFromCurrentJsonPath() {
	if (!currentJsonPathList.empty()) {
		Node &lastNodeOfCurrentJsonPath = currentJsonPathList.back();
		if (lastNodeOfCurrentJsonPath.outputOfETFUptoThisNodeInCurrentPathHasAcceptState) {
			countOfPrefixPathsThatsETFOutputContainsAcceptState--;
		}
		lastNodeOfCurrentJsonPath.clearAutomationStates();
		currentJsonPathList.pop_back();
	}
}

void pushKeyNodeInCurrentJsonPathList(const string &key) {
	currentJsonPathList.emplace_back(key, true);
	computeExtendedTransitionFunctionForCurrentJsonPath();
}

void pushIndexNodeInCurrentJsonPath(int index) {
	currentJsonPathList.emplace_back(index);
}

void incrementIndexOfLastNodeInCurrentJsonPath() {
	Node &lastNodeOfCurrentJsonPath = currentJsonPathList.back();
	if (lastNodeOfCurrentJsonPath.outputOfETFUptoThisNodeInCurrentPathHasAcceptState) {
		countOfPrefixPathsThatsETFOutputContainsAcceptState--;
	}
	lastNodeOfCurrentJsonPath.indexValue++;
	computeExtendedTransitionFunctionForCurrentJsonPath();
}

void jsonStreamingEventHandlerForJsonPathQuery(const JsonStreamEvent<string> &jsonStreamEvent) {
	StreamToken streamToken = jsonStreamEvent.getStreamToken();
	JsonTokenType streamTokenType = streamToken.tokenType;
	if (streamTokenType == KEY_TOKEN) {
		if (isJsonPathQuerySatisfy()) {
			updateResult(streamToken);
		}
		pushKeyNodeInCurrentJsonPathList(streamToken.value);
	} else if (streamTokenType == VALUE_TOKEN) {
		if (isLastNodeOfCurrentJsonPathIsIndexNode() && isIncrementIndexOfLastNodeIsNecessaryNowConsideringETFState()) {
			incrementIndexOfLastNodeInCurrentJsonPath();
		}
		if (isJsonPathQuerySatisfy()) {
			updateResult(streamToken);
		}
		if (isLastNodeOfCurrentJsonPathIsKeyNode()) {
			popNodeFromCurrentJsonPath();
		}
	} else if (streamTokenType == OBJECT_STARTED_TOKEN) {
		if (isLastNodeOfCurrentJsonPathIsIndexNode() && isIncrementIndexOfLastNodeIsNecessaryNowConsideringETFState()) {
			incrementIndexOfLastNodeInCurrentJsonPath();
		}
		if (isJsonPathQuerySatisfy()) {
			updateResult(streamToken);
		}
	} else if (streamTokenType == LIST_STARTED_TOKEN) {
		if (isLastNodeOfCurrentJsonPathIsIndexNode() && isIncrementIndexOfLastNodeIsNecessaryNowConsideringETFState()) {
			incrementIndexOfLastNodeInCurrentJsonPath();
		}
		if (isJsonPathQuerySatisfy()) {
			updateResult(streamToken);
		}
		pushIndexNodeInCurrentJsonPath(-1);
	} else if (streamTokenType == LIST_ENDED_TOKEN) {
		popNodeFromCurrentJsonPath();
		if (isJsonPathQuerySatisfy()) {
			updateResult(streamToken);
		}
		if (isLastNodeOfCurrentJsonPathIsKeyNode()) {
			popNodeFromCurrentJsonPath();
		}
	} else if (streamTokenType == OBJECT_ENDED_TOKEN) {
		if (isJsonPathQuerySatisfy()) {
			updateResult(streamToken);
		}
		if (isLastNodeOfCurrentJsonPathIsKeyNode()) {
			popNodeFromCurrentJsonPath();
		}
	} else if (streamTokenType == DOCUMENT_END_TOKEN) {
		printJsonPathQueryResult();
	}
}

/*
 * This function will tokenize and process the json path query for implementation purpose
 */
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
		/**
		 * $.user.details[0].name
		 * in that case for each iteration stringBetweenTwoConsecutiveDot will be
		 * user
		 * details[0]
		 * name
		 */
		int ithOccurrenceOfDot = occurrenceIndexesOfDotInQuery[i] + 1;
		int iPlusOneOccurrenceOfDot = jsonPathQuery.size() - 1;
		if (i + 1 <= occurrenceIndexesOfDotInQuery.size() - 1) {
			iPlusOneOccurrenceOfDot = occurrenceIndexesOfDotInQuery[i + 1] - 1;
		}
		string stringBetweenTwoConsecutiveDot;
		if (ithOccurrenceOfDot <= iPlusOneOccurrenceOfDot) {
			stringBetweenTwoConsecutiveDot =
				jsonPathQuery.substr(ithOccurrenceOfDot, iPlusOneOccurrenceOfDot - ithOccurrenceOfDot + 1);
		}
		indexOfNextLeftSquareBracket = stringBetweenTwoConsecutiveDot.find('[');
		int indexOfStartOfFirstIndexNode = -1;
		/**
		 * This step will further split value of stringBetweenTwoConsecutiveDot
		 * details[0] will split into details(keyNode) and 0(indexNode)
		 * user and name string wont split further
		 */
		while (indexOfNextLeftSquareBracket != string::npos) {
			indexOfNextRightSquareBracket = stringBetweenTwoConsecutiveDot.find(']', indexOfNextLeftSquareBracket + 1);
			if (indexOfNextRightSquareBracket != string::npos) {
				if (indexOfStartOfFirstIndexNode == -1) {
					indexOfStartOfFirstIndexNode = indexOfNextLeftSquareBracket;
					jsonPathQueryTokenized.emplace_back(stringBetweenTwoConsecutiveDot.substr(0,
																							  indexOfNextLeftSquareBracket),
														true);
				}
				jsonPathQueryTokenized.emplace_back(stringBetweenTwoConsecutiveDot.substr(
					indexOfNextLeftSquareBracket + 1,
					(indexOfNextRightSquareBracket - 1)
						- (indexOfNextLeftSquareBracket + 1) + 1), false);
				indexOfNextLeftSquareBracket =
					stringBetweenTwoConsecutiveDot.find('[', indexOfNextRightSquareBracket + 1);
			} else break;
		}
		if (indexOfStartOfFirstIndexNode == -1) {
			jsonPathQueryTokenized.emplace_back(stringBetweenTwoConsecutiveDot, true);
		}
	}
	for (auto &token : jsonPathQueryTokenized) {
		wildCardOrRecursiveDescentContainsInQuery =
			wildCardOrRecursiveDescentContainsInQuery || (token.recursiveDescent || token.wildcard);
	}

	/**
	 * after splitting $.user..name.*
	 * jsonPathQueryTokenized = [{"keyValue": "$"},{"recursive descent": true, "keyValue":""}, {"keyValue":"user"}, {"keyValue":"name"}, {"keyValue":"wildcard"}]
	 * This iteration will be processed this into
	 * jsonPathQueryProcessed = [{"keyValue": "$"},{"recursive descent": true, "keyValue":"user"}, {"keyValue":"name"}, {"keyValue":"wildcard"}]
	 */
	for (int i = 0; i < jsonPathQueryTokenized.size(); i++) {
		Node &jsonQueryNode = jsonPathQueryTokenized.at(i);
		if (jsonPathQueryTokenized.at(i).recursiveDescent) {
			jsonPathQueryTokenized[i + 1].recursiveDescent = true;
			i++;
		}
		jsonPathQueryProcessed.emplace_back(jsonPathQueryTokenized.at(i));
	}
}
/**
 * Initializing node for $ and processing the JsonPathQuery
 * @param jsonPathQuery
 */
void initJsonPathQueryStates(string &jsonPathQuery) {
	Node node("$", true);
	if (jsonPathQuery == "$") {
		node.outputOfETFUptoThisNodeInCurrentPathHasAcceptState = true;
	} else {
		node.outputOfETFUptoThisNodeInCurrentPathExceptAcceptState.insert(0);
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
	jsonStreamParser.setEventHandler(jsonStreamingEventHandlerForJsonPathQuery);
	jsonStreamParser.startJsonStreaming(fileName);
}

int main(int argc, char **argv) {
	freopen("output.txt", "a+", stdout);
	JsonStreamParser jsonStreamParser = JsonStreamParser();
	string fileName(argv[1]);
	string jsonPathQuery(argv[2]);
	executeJsonPathQuery(fileName, jsonPathQuery);
}