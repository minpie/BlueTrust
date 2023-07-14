/*
related_JSON.cpp

JSON 관련
*/
#ifndef _DEFINED_ABOUT_JSON_
#define _DEFINED_ABOUT_JSON_
#include <ArduinoJson.h>


// Functions:
bool convertToJson(const String& t, JsonVariant variant) {
  char buf[128];
  t.toCharArray(buf, 128);
  return variant.set(t);
}
DeserializationError InterpretJson(DynamicJsonDocument * obj_jdoc, char * text_jdoc){
    DeserializationError err = deserializeJson(*obj_jdoc, text_jdoc);
    return err;
}


#endif
