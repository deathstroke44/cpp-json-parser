# stream-json-c++

**Usage**

Create a new Json Stream Parser.

```cpp
Json_stream_parser json_stream_parser = Json_stream_parser();
```
Event dispatcher emit events after parsing a token from json and token type can be one of this:
```cpp
[
    KEY_EVENT, // Key found
    VALUE_EVENT, // value found
    OBJECT_LIST_EVENT, // Object/list started/ended
    Document_END // Document ended
]
```

Set event handler function for that json stream parser

```cpp
json_stream_parser.setEventHandler(handleEvent);
```
A dummy handler function for new token found event
```cpp

void handleEvent(const JsonStreamEvent<string>& event) {
    current_event = event;
    // handle event as your need. I have just added this code for testing and demo purpose.
    int prev_key_found = key_found;
    StreamToken streamToken = event.getStreamToken();
    if (key_found == 0 || key_found ==1)
    {
        if(streamToken.token_type == JsonEventType::KEY_EVENT) { push_string_key(streamToken.value);}
        if (streamToken.token_type == VALUE_EVENT) {
            handle_value_found();

        }
        if(streamToken.token_type == JsonEventType::OBJECT_LIST_EVENT) {
            if (streamToken.value == "list started") {
                set_part_of_value("list");
                set_new_list_started();
            }
            else if (streamToken.value == "list ended") {
                set_last_list_ended();
            }
            else if (streamToken.value == "object started") {
                if (get_part_of_value() == "list") {
                    set_new_value_added_in_list();
                }
                set_part_of_value("object");
            }
            else if (streamToken.value == "object ended") {
                set_last_object_ended();
            }

        }
        if(key_found == 1 && prev_key_found == 1) {
            addEventInDesiredResult(current_event.getStreamToken());
        }
        else if(key_found == 2 && prev_key_found != 2) {
            remove_last_delimeter();
            json_stream_parser.stop_emitting_event = true;
            std::cout << "Got value of desired key: " << desired_key<<endl<<traversedJson<<endl;
        }
    }
}
```



After subscribing to the required events and adding event handlers, start parsing by giving the relative path of the JSON file to the parsing function:

```cpp
json_stream_parser.start_json_streaming(fileName);
```

Getting a Document_END event indicates that the parser has already scanned the whole JSON file.

**[Demo.cpp](https://github.com/deathstroke44/cpp-json-parser/blob/main/src/demo.cpp) file will give you a better idea of how to use this json stream parser.**

**
[tests folder](https://github.com/deathstroke44/cpp-json-parser/tree/main/src/tests) contains some JSON files and a [python script](https://github.com/deathstroke44/cpp-json-parser/blob/main/src/tests/tests.py) that will run some test cases of my JSON path query code.  
**
