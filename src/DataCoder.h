//  DataCoder.h - GoFarmTech Framework Source Code
//  Author: bratello

#ifndef DATA_CODER_H
#define DATA_CODER_H
#include    <Arduino.h>

class DataCoder {
public:
    static String encode(const String& data);
    static String decode(const String& data);
};

#endif