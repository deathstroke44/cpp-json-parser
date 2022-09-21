#ifndef EVENT_HH__
#define EVENT_HH__
#include <string>
#include<bits/stdc++.h>
using namespace std;

enum TokenType{
    OPERATOR_TOKEN,
    STRING_TOKEN,
    NUMBER_TOKEN,
    EXP_TOKEN,
    FLOAT_TOKEN,
    INTEGER_TOKEN,
    BOOLEAN_TOKEN,
    NULL_TOKEN    
};

enum JsonEventType {
    KEY_EVENT,
    STRING_EVENT,
    BOOL_EVENT,
    NULL_EVENT,
    INTEGER_EVENT,
    FLOAT_EVENT,
    EXPONENT_EVENT,
    OBJECT_LIST_EVENT,
    OBJECT_STARTED,
    OBJECT_ENDED,
    LIST_STARTED,
    LIST_ENDED,
    Document_END,
    VALUE_EVENT,
};

class StreamToken {
  public:
    JsonEventType tokenType;
    JsonEventType tokenSubType;
    bool isDefault = true;
    string value;
    StreamToken(JsonEventType  _token_type, JsonEventType  _token_sub_type, string _value) : tokenType(_token_type), tokenSubType(_token_sub_type), value(_value), isDefault(false) {};
    StreamToken() = default;

};

/**
  @class Event
  @brief Base class for all events

  This class provides the interface that needs to be implemented by an event.
  Each dependent class is required to implement type() to uniquely identify
  events. As a convenience, each class should have a static descriptor so that
  clients may refer to it without having to create an instance.

  Unfortunately, I have no idea how to enforce the existence of this attribute.
*/

template <typename T>
class JsonStreamEvent{
protected:
  T _type;
  StreamToken stream_token;
  bool _handled = false;
public:
  JsonStreamEvent() = default;
  JsonStreamEvent(T type, const StreamToken& name) : _type(type), stream_token(name){};
  inline const string type() const { return _type;};
  inline const StreamToken& getStreamToken() const { return stream_token;};
  virtual bool isHandled(){ return _handled;};
private:
};
#endif
