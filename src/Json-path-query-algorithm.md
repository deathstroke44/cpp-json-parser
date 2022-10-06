PROCESS-RESULT():
    result = AppendResultsByCommaSeparators()
    if multipleResultQuery:
        result = "[" + result + "]"
    return result

UPDATE-RESULT(streamToken, key):
    resultUpToNowForThisKey = jsonPathQueryResultKeys.get(key)
    lastAddedToken = jsonPathQueryResultsLastAddedTokenMap.get(key)
    if appendingDelimiterNeeded(streamToken, lastAddedToken):
        lastAddedToken.append(",")
    lastAddedToken.append(getProcessedValue(streamToken));

UPDATE-RESULT-IF-NEEDED(streamToken, ignoreEventFlag, shouldAddThisEvent, previousKeySatisfy, previousKey):
    currentKeySatisfy = isCurrentJsonPathMatchJsonPathQuery();
    if currentKeyMatched and not ignoreEventFlag:
        currentKey = getCurrentJsonPathUptoQueryLength()
        UPDATE-RESULT(streamToken, currentKey)
    else if (not currentKeySatisfy and shouldAddThisEvent and previousKeySatisfy)
        UPDATE-RESULT(streamToken, previousKey)

PROCESS-STREAM-TOKEN-AND-UPDATE-STATES(streamToken, shouldAddThisEvent, ignoreEventFlag)
    if streamToken.type == KEY_TOKEN:
        key=Key(isStringKey=false,index=-streamToken.value)
        ignoreEventFlag = ignoreEventFlag  !currentJsonPathMatchJsonPathQuery()
        currentJsonPathStack.push({isStringKey:true, key: streamToken.value})
    else if streamToken.type == VALUE_TOKEN:
        if PartOfObject:
            currentJsonPathStack.pop()
            shouldAddThisEvent = true
        else if PartOfList:
            currentJsonPathStack.top().index++
    else if streamToken.type == LIST_STARTED_TOKEN:
        IF PartOfList:
            currentJsonPathStack.top().index++
        currentJsonPathStack.push({isStringKey=false,index=-1})
        traversingListOrObjectStack.push("list")
    else if streamToken.type == LIST_ENDED_TOKEN:
        currentJsonPathStack.pop()
        if PartOfObject:
            currentJsonPathStack.pop()
        shouldAddThisEvent=isCurrentTokenLastCharacterOfKeysValue();
    else if streamToken.type == OBJECT_STARTED_TOKEN:
        if PartOfList:
            currentJsonPathStack.top().index++
        traversingListOrObjectStack.push("object")
    else if streamToken.type == OBJECT_ENDED_TOKEN:
        traversingListOrObjectStack.pop()
        if PartOfObject:
            currentJsonPathStack.pop()
        shouldAddThisEvent=true

HANDLE-EVENT(streamToken):
    IF streamToken.tokenType == DOCUMENT_END:
        PROCESS-RESULT()
        RETURN
    shouldAddThisEvent = ignoreEventFlag = false;
    previousKeySatisfy = isCurrentJsonPathMatchJsonPathQuery();
    PROCESS-STREAM-TOKEN-AND-UPDATE-STATES(streamToken, shouldAddThisEvent, ignoreEventFlag);
    UPDATE-RESULT-IF-NEEDED(streamToken, ignoreEventFlag, shouldAddThisEvent, previousKeySatisfy, previousKey);