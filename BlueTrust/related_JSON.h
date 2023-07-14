/*
related_JSON.h

JSON 관련
*/
#ifndef _DEFINED_ABOUT_JSON_
#define _DEFINED_ABOUT_JSON_
#include <ArduinoJson.h>


// Functions:
bool convertToJson(const String& t, JsonVariant variant);
DeserializationError InterpretJson(DynamicJsonDocument * obj_jdoc, char * text_jdoc);
#endif
