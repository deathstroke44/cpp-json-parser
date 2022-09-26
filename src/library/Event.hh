#ifndef EVENT_HH__
#define EVENT_HH__

#include <string>
#include<bits/stdc++.h>
#include "Custom-data-types.hh"

using namespace std;


/**
  @class Event
  @brief Base class for all events

  This class provides the interface that needs to be implemented by an event.
  Each dependent class is required to implement type() to uniquely identify
  events. As a convenience, each class should have a static descriptor so that
  clients may refer to it without having to create an instance.

  Unfortunately, I have no idea how to enforce the existence of this attribute.
*/

template<typename T>
class JsonStreamEvent {
protected:
    T _type;
    StreamToken stream_token;
    bool _handled = false;
public:
    JsonStreamEvent() = default;

    JsonStreamEvent(T type, const StreamToken &name) : _type(type), stream_token(name) {};

    inline const string type() const { return _type; };

    inline const StreamToken &getStreamToken() const { return stream_token; };

    virtual bool isHandled() { return _handled; };
private:
};

#endif
