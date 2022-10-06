JSON path query:$..user -filename: example.json</br>

| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 1, value: object}**|
|jsonPathKeyPrev: |**[{key: "$"}]**|
|jsonPathKey: |**[{key: "$"}]**|
|objectListStackPrev: |**[]**|
|objectListStack: |**[object]**|
|currentKeyMatched: |**false**|
|previousKeyValid: |**false**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**false**|
|needToUpdateResult: |**false**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 0, value: case_1}**|
|jsonPathKeyPrev: |**[{key: "$"}]**|
|jsonPathKey: |**[{key: "$"}, {key: "case_1"}]**|
|objectListStackPrev: |**[object]**|
|objectListStack: |**[object]**|
|currentKeyMatched: |**false**|
|previousKeyValid: |**false**|
|ignoreEventFlag: |**true**|
|shouldAddThisEvent: |**false**|
|needToUpdateResult: |**false**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 1, value: object}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_1"}]**|
|jsonPathKey: |**[{key: "$"}, {key: "case_1"}]**|
|objectListStackPrev: |**[object]**|
|objectListStack: |**[object, object]**|
|currentKeyMatched: |**false**|
|previousKeyValid: |**false**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**false**|
|needToUpdateResult: |**false**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 0, value: user}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_1"}]**|
|jsonPathKey: |**[{key: "$"}, {key: "case_1"}, {key: "user"}]**|
|objectListStackPrev: |**[object, object]**|
|objectListStack: |**[object, object]**|
|currentKeyMatched: |**true**|
|previousKeyValid: |**false**|
|ignoreEventFlag: |**true**|
|shouldAddThisEvent: |**false**|
|needToUpdateResult: |**false**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 1, value: object}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_1"}, {key: "user"}]**|
|jsonPathKey: |**[{key: "$"}, {key: "case_1"}, {key: "user"}]**|
|objectListStackPrev: |**[object, object]**|
|objectListStack: |**[object, object, object]**|
|currentKeyMatched: |**true**|
|previousKeyValid: |**true**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**false**|
|needToUpdateResult: |**true**|
|updateKey: |**$.case_1.user**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 0, value: name}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_1"}, {key: "user"}]**|
|jsonPathKey: |**[{key: "$"}, {key: "case_1"}, {key: "user"}, {key: "name"}]**|
|objectListStackPrev: |**[object, object, object]**|
|objectListStack: |**[object, object, object]**|
|currentKeyMatched: |**true**|
|previousKeyValid: |**true**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**false**|
|needToUpdateResult: |**true**|
|updateKey: |**$.case_1.user**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 6, value: Jason}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_1"}, {key: "user"}, {key: "name"}]**|
|jsonPathKey: |**[{key: "$"}, {key: "case_1"}, {key: "user"}]**|
|objectListStackPrev: |**[object, object, object]**|
|objectListStack: |**[object, object, object]**|
|currentKeyMatched: |**true**|
|previousKeyValid: |**true**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**true**|
|needToUpdateResult: |**true**|
|updateKey: |**$.case_1.user**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 0, value: lang}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_1"}, {key: "user"}]**|
|jsonPathKey: |**[{key: "$"}, {key: "case_1"}, {key: "user"}, {key: "lang"}]**|
|objectListStackPrev: |**[object, object, object]**|
|objectListStack: |**[object, object, object]**|
|currentKeyMatched: |**true**|
|previousKeyValid: |**true**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**false**|
|needToUpdateResult: |**true**|
|updateKey: |**$.case_1.user**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 3, value: list}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_1"}, {key: "user"}, {key: "lang"}]**|
|jsonPathKey: |**[{key: "$"}, {key: "case_1"}, {key: "user"}, {key: "lang"}, {index: -1}]**|
|objectListStackPrev: |**[object, object, object]**|
|objectListStack: |**[object, object, object, list]**|
|currentKeyMatched: |**true**|
|previousKeyValid: |**true**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**false**|
|needToUpdateResult: |**true**|
|updateKey: |**$.case_1.user**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 6, value: python}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_1"}, {key: "user"}, {key: "lang"}, {index: -1}]**|
|jsonPathKey: |**[{key: "$"}, {key: "case_1"}, {key: "user"}, {key: "lang"}, {index: 0}]**|
|objectListStackPrev: |**[object, object, object, list]**|
|objectListStack: |**[object, object, object, list]**|
|currentKeyMatched: |**true**|
|previousKeyValid: |**true**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**false**|
|needToUpdateResult: |**true**|
|updateKey: |**$.case_1.user**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 3, value: list}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_1"}, {key: "user"}, {key: "lang"}, {index: 0}]**|
|jsonPathKey: |**[{key: "$"}, {key: "case_1"}, {key: "user"}, {key: "lang"}, {index: 1}, {index: -1}]**|
|objectListStackPrev: |**[object, object, object, list]**|
|objectListStack: |**[object, object, object, list, list]**|
|currentKeyMatched: |**true**|
|previousKeyValid: |**true**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**false**|
|needToUpdateResult: |**true**|
|updateKey: |**$.case_1.user**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 6, value: c}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_1"}, {key: "user"}, {key: "lang"}, {index: 1}, {index: -1}]**|
|jsonPathKey: |**[{key: "$"}, {key: "case_1"}, {key: "user"}, {key: "lang"}, {index: 1}, {index: 0}]**|
|objectListStackPrev: |**[object, object, object, list, list]**|
|objectListStack: |**[object, object, object, list, list]**|
|currentKeyMatched: |**true**|
|previousKeyValid: |**true**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**false**|
|needToUpdateResult: |**true**|
|updateKey: |**$.case_1.user**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 6, value: c++}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_1"}, {key: "user"}, {key: "lang"}, {index: 1}, {index: 0}]**|
|jsonPathKey: |**[{key: "$"}, {key: "case_1"}, {key: "user"}, {key: "lang"}, {index: 1}, {index: 1}]**|
|objectListStackPrev: |**[object, object, object, list, list]**|
|objectListStack: |**[object, object, object, list, list]**|
|currentKeyMatched: |**true**|
|previousKeyValid: |**true**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**false**|
|needToUpdateResult: |**true**|
|updateKey: |**$.case_1.user**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 4, value: list}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_1"}, {key: "user"}, {key: "lang"}, {index: 1}, {index: 1}]**|
|jsonPathKey: |**[{key: "$"}, {key: "case_1"}, {key: "user"}, {key: "lang"}, {index: 1}]**|
|objectListStackPrev: |**[object, object, object, list, list]**|
|objectListStack: |**[object, object, object, list]**|
|currentKeyMatched: |**true**|
|previousKeyValid: |**true**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**false**|
|needToUpdateResult: |**true**|
|updateKey: |**$.case_1.user**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 4, value: list}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_1"}, {key: "user"}, {key: "lang"}, {index: 1}]**|
|jsonPathKey: |**[{key: "$"}, {key: "case_1"}, {key: "user"}]**|
|objectListStackPrev: |**[object, object, object, list]**|
|objectListStack: |**[object, object, object]**|
|currentKeyMatched: |**true**|
|previousKeyValid: |**true**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**false**|
|needToUpdateResult: |**true**|
|updateKey: |**$.case_1.user**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 0, value: address}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_1"}, {key: "user"}]**|
|jsonPathKey: |**[{key: "$"}, {key: "case_1"}, {key: "user"}, {key: "address"}]**|
|objectListStackPrev: |**[object, object, object]**|
|objectListStack: |**[object, object, object]**|
|currentKeyMatched: |**true**|
|previousKeyValid: |**true**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**false**|
|needToUpdateResult: |**true**|
|updateKey: |**$.case_1.user**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 1, value: object}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_1"}, {key: "user"}, {key: "address"}]**|
|jsonPathKey: |**[{key: "$"}, {key: "case_1"}, {key: "user"}, {key: "address"}]**|
|objectListStackPrev: |**[object, object, object]**|
|objectListStack: |**[object, object, object, object]**|
|currentKeyMatched: |**true**|
|previousKeyValid: |**true**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**false**|
|needToUpdateResult: |**true**|
|updateKey: |**$.case_1.user**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 0, value: country}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_1"}, {key: "user"}, {key: "address"}]**|
|jsonPathKey: |**[{key: "$"}, {key: "case_1"}, {key: "user"}, {key: "address"}, {key: "country"}]**|
|objectListStackPrev: |**[object, object, object, object]**|
|objectListStack: |**[object, object, object, object]**|
|currentKeyMatched: |**true**|
|previousKeyValid: |**true**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**false**|
|needToUpdateResult: |**true**|
|updateKey: |**$.case_1.user**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 6, value: Bangladesh}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_1"}, {key: "user"}, {key: "address"}, {key: "country"}]**|
|jsonPathKey: |**[{key: "$"}, {key: "case_1"}, {key: "user"}, {key: "address"}]**|
|objectListStackPrev: |**[object, object, object, object]**|
|objectListStack: |**[object, object, object, object]**|
|currentKeyMatched: |**true**|
|previousKeyValid: |**true**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**true**|
|needToUpdateResult: |**true**|
|updateKey: |**$.case_1.user**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 0, value: city}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_1"}, {key: "user"}, {key: "address"}]**|
|jsonPathKey: |**[{key: "$"}, {key: "case_1"}, {key: "user"}, {key: "address"}, {key: "city"}]**|
|objectListStackPrev: |**[object, object, object, object]**|
|objectListStack: |**[object, object, object, object]**|
|currentKeyMatched: |**true**|
|previousKeyValid: |**true**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**false**|
|needToUpdateResult: |**true**|
|updateKey: |**$.case_1.user**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 6, value: Dhaka}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_1"}, {key: "user"}, {key: "address"}, {key: "city"}]**|
|jsonPathKey: |**[{key: "$"}, {key: "case_1"}, {key: "user"}, {key: "address"}]**|
|objectListStackPrev: |**[object, object, object, object]**|
|objectListStack: |**[object, object, object, object]**|
|currentKeyMatched: |**true**|
|previousKeyValid: |**true**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**true**|
|needToUpdateResult: |**true**|
|updateKey: |**$.case_1.user**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 2, value: object}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_1"}, {key: "user"}, {key: "address"}]**|
|jsonPathKey: |**[{key: "$"}, {key: "case_1"}, {key: "user"}]**|
|objectListStackPrev: |**[object, object, object, object]**|
|objectListStack: |**[object, object, object]**|
|currentKeyMatched: |**true**|
|previousKeyValid: |**true**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**true**|
|needToUpdateResult: |**true**|
|updateKey: |**$.case_1.user**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 2, value: object}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_1"}, {key: "user"}]**|
|jsonPathKey: |**[{key: "$"}, {key: "case_1"}]**|
|objectListStackPrev: |**[object, object, object]**|
|objectListStack: |**[object, object]**|
|currentKeyMatched: |**false**|
|previousKeyValid: |**true**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**true**|
|needToUpdateResult: |**true**|
|updateKey: |**$.case_1.user**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 2, value: object}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_1"}]**|
|jsonPathKey: |**[{key: "$"}]**|
|objectListStackPrev: |**[object, object]**|
|objectListStack: |**[object]**|
|currentKeyMatched: |**false**|
|previousKeyValid: |**false**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**true**|
|needToUpdateResult: |**false**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 0, value: case_2}**|
|jsonPathKeyPrev: |**[{key: "$"}]**|
|jsonPathKey: |**[{key: "$"}, {key: "case_2"}]**|
|objectListStackPrev: |**[object]**|
|objectListStack: |**[object]**|
|currentKeyMatched: |**false**|
|previousKeyValid: |**false**|
|ignoreEventFlag: |**true**|
|shouldAddThisEvent: |**false**|
|needToUpdateResult: |**false**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 1, value: object}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_2"}]**|
|jsonPathKey: |**[{key: "$"}, {key: "case_2"}]**|
|objectListStackPrev: |**[object]**|
|objectListStack: |**[object, object]**|
|currentKeyMatched: |**false**|
|previousKeyValid: |**false**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**false**|
|needToUpdateResult: |**false**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 0, value: user}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_2"}]**|
|jsonPathKey: |**[{key: "$"}, {key: "case_2"}, {key: "user"}]**|
|objectListStackPrev: |**[object, object]**|
|objectListStack: |**[object, object]**|
|currentKeyMatched: |**true**|
|previousKeyValid: |**false**|
|ignoreEventFlag: |**true**|
|shouldAddThisEvent: |**false**|
|needToUpdateResult: |**false**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 3, value: list}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_2"}, {key: "user"}]**|
|jsonPathKey: |**[{key: "$"}, {key: "case_2"}, {key: "user"}, {index: -1}]**|
|objectListStackPrev: |**[object, object]**|
|objectListStack: |**[object, object, list]**|
|currentKeyMatched: |**true**|
|previousKeyValid: |**true**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**false**|
|needToUpdateResult: |**true**|
|updateKey: |**$.case_2.user**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 6, value: Mathew}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_2"}, {key: "user"}, {index: -1}]**|
|jsonPathKey: |**[{key: "$"}, {key: "case_2"}, {key: "user"}, {index: 0}]**|
|objectListStackPrev: |**[object, object, list]**|
|objectListStack: |**[object, object, list]**|
|currentKeyMatched: |**true**|
|previousKeyValid: |**true**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**false**|
|needToUpdateResult: |**true**|
|updateKey: |**$.case_2.user**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 4, value: list}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_2"}, {key: "user"}, {index: 0}]**|
|jsonPathKey: |**[{key: "$"}, {key: "case_2"}]**|
|objectListStackPrev: |**[object, object, list]**|
|objectListStack: |**[object, object]**|
|currentKeyMatched: |**false**|
|previousKeyValid: |**true**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**true**|
|needToUpdateResult: |**true**|
|updateKey: |**$.case_2.user**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 2, value: object}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_2"}]**|
|jsonPathKey: |**[{key: "$"}]**|
|objectListStackPrev: |**[object, object]**|
|objectListStack: |**[object]**|
|currentKeyMatched: |**false**|
|previousKeyValid: |**false**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**true**|
|needToUpdateResult: |**false**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|processingToken: |**{type: 2, value: object}**|
|jsonPathKeyPrev: |**[{key: "$"}]**|
|jsonPathKey: |**[{key: "$"}]**|
|objectListStackPrev: |**[object]**|
|objectListStack: |**[]**|
|currentKeyMatched: |**false**|
|previousKeyValid: |**false**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**true**|
|needToUpdateResult: |**false**|
</br></br>
All answers: .$.case_1.user -> {"name" : "Jason","lang" : ["python",["c","c++"]],"address" : {"country" : "Bangladesh","city" : "Dhaka"}}
All answers: .$.case_2.user -> ["Mathew"]
Got value of desired key final result:
[{"name" : "Jason","lang" : ["python",["c","c++"]],"address" : {"country" : "Bangladesh","city" : "Dhaka"}},["Mathew"]]
