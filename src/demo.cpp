#include "code.hh"
using namespace std;

void handleEvent(const Event<JsonEventType>& event) {
    // handle event as your need. I have just added this code for testing and demo purpose.
    {
        if(event.type() == JsonEventType::KEY_EVENT) { std::cout << "EMITTED Event TYPE: KEY " << event.getName() << std::endl;}
        if(event.type() == JsonEventType::STRING_EVENT) { std::cout << "EMITTED Event TYPE: String " << event.getName() << std::endl;}
        if(event.type() == JsonEventType::BOOL_EVENT) { std::cout << "EMITTED Event TYPE: BOOL " << event.getName() << std::endl;}
        if(event.type() == JsonEventType::NULL_EVENT) { std::cout << "EMITTED Event TYPE: NULL " << event.getName() << std::endl;}
        if(event.type() == JsonEventType::INTEGER_EVENT) { std::cout << "EMITTED Event TYPE: INTEGER " << event.getName() << std::endl;}
        if(event.type() == JsonEventType::FLOAT_EVENT) { std::cout << "EMITTED Event TYPE: FLOAT " << event.getName() << std::endl;}
        if(event.type() == JsonEventType::EXPONENT_EVENT) { std::cout << "EMITTED Event TYPE: EXPONENT " << event.getName() << std::endl;}
        if(event.type() == JsonEventType::OBJECT_LIST_EVENT) { std::cout << "EMITTED Event TYPE: OBJECT LIST EVENT " << event.getName() << std::endl;}
        if(event.type() == JsonEventType::Document_END) { std::cout << "EMITTED Event TYPE: Full Document Read Completed " << std::endl;}
        cout<<"---------------------------------------------------------------------------------"<<endl;
    }
    
}

int main() {
    Json_stream_parser json_stream_parser = Json_stream_parser();
    string fileName = "code-test.json";
    // subscribe to the events those are needed for your purpose
    // Subscribing to all events is not necessary. 
    // There is no constraint that you have to use same fuction for all events
    {
        json_stream_parser.eventDispatcher.subscribe( JsonEventType::KEY_EVENT, handleEvent );
        json_stream_parser.eventDispatcher.subscribe( JsonEventType::STRING_EVENT, handleEvent );
        json_stream_parser.eventDispatcher.subscribe( JsonEventType::BOOL_EVENT, handleEvent );
        json_stream_parser.eventDispatcher.subscribe( JsonEventType::NULL_EVENT, handleEvent );
        json_stream_parser.eventDispatcher.subscribe( JsonEventType::INTEGER_EVENT, handleEvent );
        json_stream_parser.eventDispatcher.subscribe( JsonEventType::FLOAT_EVENT, handleEvent );
        json_stream_parser.eventDispatcher.subscribe( JsonEventType::EXPONENT_EVENT, handleEvent );
        json_stream_parser.eventDispatcher.subscribe( JsonEventType::OBJECT_LIST_EVENT, handleEvent );
        json_stream_parser.eventDispatcher.subscribe( JsonEventType::Document_END, handleEvent );
    }

    json_stream_parser.start_json_streaming(fileName);
}