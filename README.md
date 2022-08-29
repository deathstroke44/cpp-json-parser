# stream-json-c++

**Usage**

Create a new Json Stream Parser.

```cpp
Json_stream_parser json_stream_parser = Json_stream_parser();
```

Event dispatcher can fire following events:
```cpp
enum JsonEventType {
    KEY_EVENT, // Key found
    STRING_EVENT, // String value found
    BOOL_EVENT, // Boolean value found
    NULL_EVENT, // null value found
    INTEGER_EVENT, // integer value found
    FLOAT_EVENT, // float value found
    EXPONENT_EVENT, // exponent value found
    OBJECT_LIST_EVENT, // Object/list started/ended
    Document_END // Document ended
};
```

Subscribe to the required events thrown by DispatcherEvent of the JSON parser. Also, it is mandatory to add a handler function for each subscribed event.

```cpp
json_stream_parser.eventDispatcher.subscribe( JsonEventType::KEY_EVENT, handleEvent );
```
A dummy handler function for new key found event
```cpp

void handleEvent(const Event<JsonEventType>& event) {
    // handle event as your need. I have just added this code for testing and demo purpose.
    std::cout << "EMITTED Event TYPE: KEY " << event.getName() << std::endl;
}
```



start parsing by giving the relative path of the JSON file to the parsing function:

```cpp
json_stream_parser.start_json_streaming(fileName);
```

**[Demo.cpp](https://github.com/deathstroke44/cpp-json-parser/blob/main/src/demo.cpp) file will give you a better idea of how to use this json stream parser.**
