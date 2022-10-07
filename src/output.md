JSON path query:$..user -filename: example.json</br>

| Variable      | Value |
| ----      | ---- |
|iteration: |**1**|
|processingToken: |**{type: OBJECT_STARTED_TOKEN, value: object}**|
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
|iteration: |**2**|
|processingToken: |**{type: KEY_TOKEN, value: case_1}**|
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
|iteration: |**3**|
|processingToken: |**{type: OBJECT_STARTED_TOKEN, value: object}**|
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
|iteration: |**4**|
|processingToken: |**{type: KEY_TOKEN, value: user}**|
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
|iteration: |**5**|
|processingToken: |**{type: OBJECT_STARTED_TOKEN, value: object}**|
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
|iteration: |**6**|
|processingToken: |**{type: KEY_TOKEN, value: name}**|
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
|iteration: |**7**|
|processingToken: |**{type: VALUE_TOKEN, value: Jason}**|
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
|iteration: |**8**|
|processingToken: |**{type: KEY_TOKEN, value: lang}**|
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
|iteration: |**9**|
|processingToken: |**{type: LIST_STARTED_TOKEN, value: list}**|
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
|iteration: |**10**|
|processingToken: |**{type: VALUE_TOKEN, value: python}**|
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
|iteration: |**11**|
|processingToken: |**{type: LIST_STARTED_TOKEN, value: list}**|
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
|iteration: |**12**|
|processingToken: |**{type: VALUE_TOKEN, value: c}**|
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
|iteration: |**13**|
|processingToken: |**{type: VALUE_TOKEN, value: c++}**|
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
|iteration: |**14**|
|processingToken: |**{type: LIST_ENDED_TOKEN, value: list}**|
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
|iteration: |**15**|
|processingToken: |**{type: LIST_ENDED_TOKEN, value: list}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_1"}, {key: "user"}, {key: "lang"}, {index: 1}]**|
|jsonPathKey: |**[{key: "$"}, {key: "case_1"}, {key: "user"}]**|
|objectListStackPrev: |**[object, object, object, list]**|
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
|iteration: |**16**|
|processingToken: |**{type: KEY_TOKEN, value: address}**|
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
|iteration: |**17**|
|processingToken: |**{type: OBJECT_STARTED_TOKEN, value: object}**|
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
|iteration: |**18**|
|processingToken: |**{type: KEY_TOKEN, value: country}**|
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
|iteration: |**19**|
|processingToken: |**{type: VALUE_TOKEN, value: Bangladesh}**|
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
|iteration: |**20**|
|processingToken: |**{type: KEY_TOKEN, value: city}**|
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
|iteration: |**21**|
|processingToken: |**{type: VALUE_TOKEN, value: Dhaka}**|
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
|iteration: |**22**|
|processingToken: |**{type: OBJECT_ENDED_TOKEN, value: object}**|
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
|iteration: |**23**|
|processingToken: |**{type: OBJECT_ENDED_TOKEN, value: object}**|
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
|iteration: |**24**|
|processingToken: |**{type: OBJECT_ENDED_TOKEN, value: object}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_1"}]**|
|jsonPathKey: |**[{key: "$"}]**|
|objectListStackPrev: |**[object, object]**|
|objectListStack: |**[object]**|
|currentKeyMatched: |**false**|
|previousKeyValid: |**false**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**false**|
|needToUpdateResult: |**false**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|iteration: |**25**|
|processingToken: |**{type: KEY_TOKEN, value: case_2}**|
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
|iteration: |**26**|
|processingToken: |**{type: OBJECT_STARTED_TOKEN, value: object}**|
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
|iteration: |**27**|
|processingToken: |**{type: KEY_TOKEN, value: user}**|
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
|iteration: |**28**|
|processingToken: |**{type: LIST_STARTED_TOKEN, value: list}**|
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
|iteration: |**29**|
|processingToken: |**{type: VALUE_TOKEN, value: Mathew}**|
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
|iteration: |**30**|
|processingToken: |**{type: LIST_ENDED_TOKEN, value: list}**|
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
|iteration: |**31**|
|processingToken: |**{type: OBJECT_ENDED_TOKEN, value: object}**|
|jsonPathKeyPrev: |**[{key: "$"}, {key: "case_2"}]**|
|jsonPathKey: |**[{key: "$"}]**|
|objectListStackPrev: |**[object, object]**|
|objectListStack: |**[object]**|
|currentKeyMatched: |**false**|
|previousKeyValid: |**false**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**false**|
|needToUpdateResult: |**false**|
</br></br>


| Variable      | Value |
| ----      | ---- |
|iteration: |**32**|
|processingToken: |**{type: OBJECT_ENDED_TOKEN, value: object}**|
|jsonPathKeyPrev: |**[{key: "$"}]**|
|jsonPathKey: |**[{key: "$"}]**|
|objectListStackPrev: |**[object]**|
|objectListStack: |**[]**|
|currentKeyMatched: |**false**|
|previousKeyValid: |**false**|
|ignoreEventFlag: |**false**|
|shouldAddThisEvent: |**false**|
|needToUpdateResult: |**false**|
</br></br>
All answers: .$.case_1.user -> {"name" : "Jason","lang" : ["python",["c","c++"]],"address" : {"country" : "Bangladesh","city" : "Dhaka"}}
All answers: .$.case_2.user -> ["Mathew"]
Got value of desired key final result:
[{"name" : "Jason","lang" : ["python",["c","c++"]],"address" : {"country" : "Bangladesh","city" : "Dhaka"}},["Mathew"]]
