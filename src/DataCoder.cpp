//  DataCoder.cpp - GoFarmTech Framework Source Code
//  Author: bratello

#include    "DataCoder.h"
#include    "Settings.h"
#include    <Base64.h>
#include    <initializer_list>

static auto secret = {'U', 'c', '4', 'g', 'd', 'C', 'l', 'z', 'D', '8', 'P', '0', 'C', 'p', 'c', 'y', 'T', 'q', 'H', 'P'};

inline String makeString(const String& data) {
    String result = "";
    for(unsigned int i = 0; i < data.length(); ++i) {
        result.concat((char)(*(secret.begin() + (i % secret.size())) ^ data.charAt(i)));
    }
    return result;
}

String DataCoder::encode(const String& data) {
    auto result = makeString(data);
    int encodedLength = Base64.encodedLength(result.length());
    char encodedString[encodedLength + 1];
    Base64.encode(encodedString, (char*)result.c_str(), result.length());
    return encodedString;
}

String DataCoder::decode(const String& data) {
    auto decodedLength = Base64.decodedLength((char*)data.c_str(), data.length());
    char decodedString[decodedLength + 1];
    Base64.decode(decodedString, (char*)data.c_str(), data.length());
    return makeString(decodedString);
}