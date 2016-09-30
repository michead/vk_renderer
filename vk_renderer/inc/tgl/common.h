#ifndef _COMMON_H_
#define _COMMON_H_

#include <string>
#include <vector>
#include <initializer_list>
#include <queue>
#include <map>
#include <set>
#include <functional>
#include <cassert>
#include <stdio.h>
#include <cstdarg>
#include <fstream>
#include <cstdio>
#include <algorithm>
#include <thread>
#include <iostream>

// bringing stand libraray objects in scope
using std::string;
using std::vector;
using std::queue;
using std::map;
using std::pair;
using std::set;
using std::function;
using namespace std::string_literals;
using std::thread;
using std::make_pair;
using std::to_string;

#if defined(_WIN32)
#undef min
#undef max
#endif

// Prints a message (printf style) and flushes the output (useful during debugging)
inline void message(const char* msg, ...) { va_list args; va_start(args, msg); vprintf(msg, args); va_end(args); fflush(stdout); }
// Prints an error message (printf style), flushes the output (useful during debugging) and asserts
inline void error(const char* msg, ...) { va_list args; va_start(args, msg); vprintf(msg, args); va_end(args);  fflush(stdout); assert(false); }
// Checks for error, prints an error message (printf style), flushes the output (useful during debugging) and asserts
inline void error_if_not(bool check, const char* msg, ...) { if(check) return; va_list args; va_start(args, msg); vprintf(msg, args); va_end(args); fflush(stdout); assert(false); }
// simple string creation (printf style) --- warning: out should be < 4096
inline string tostring(const char* msg, ...) { char buf[4096]; va_list args; va_start(args, msg); vsprintf(buf, msg, args); va_end(args); return string(buf); }

// Python-style range: iterates from min to max in range-based for loops
struct range {
    int min = 0; // start
    int max = 1; // end
    
    // iterator
    struct iterator {
        int current; // current value
        iterator(int value) : current(value) { } // constructor
        int& operator*() { return current; } // iterator support
        iterator& operator++() { ++current; return *this; } // iterator support
        bool operator!=(const iterator& other) { return current != other.current; } // iterator support
    };
    
    // constructor for range in [0,max)
    range(int max) : max(max) { }
    // constructor for range in [min,max)
    range(int min, int max) : min(min), max(max) { }
    
    // iterator support
    iterator begin() { return iterator(min); }
    // iterator support
    iterator end() { return iterator(max); }
};

// simple implementation of optional value
// use only for small classes since it is not optimized for speed
template<typename T>
struct optional {
    bool        ok = false; // whether it exists
    T           value;      // the actual value
    
    // optional initialization
    optional() : ok(false), value() { }
    optional(const T& v): ok(true), value(v) { }
    
    // check whether it is defined
    operator bool() const { return ok; }
};

// makes an optional
template<typename T>
inline optional<T> make_optional(const T& v) { return {v}; }

// python-like list and dictionary manipulation
template<typename T>
inline vector<T> operator+(const vector<T>& a, const vector<T>& b) { auto ret = vector<T>(); ret.insert(ret.end(),a.begin(),a.end()); ret.insert(ret.end(),b.begin(),b.end()); return ret; }
template<typename T> inline vector<T>& operator+=(vector<T>& a, const vector<T>& b) { a.insert(a.end(),b.begin(),b.end()); return a; }
template<typename T> inline vector<T>& operator+=(vector<T>& a, const T& b) { a.push_back(b); return a; }
// check if a container contains an element
inline bool contains(const string& a, const string& b) { return a.find(b) != string::npos; }
template<typename T> inline bool contains(const vector<T>& a, const T& b) { return std::find(a.begin(),a.end(),b) != a.end(); }
template<typename T> inline bool contains(const set<T>& a, const T& b) { return a.find(b) != a.end(); }
template<typename T1, typename T2> inline bool contains(const map<T1,T2>& a, const T1& b) { return a.find(b) != a.end(); }
// python maps and comprehension
template<typename func>
inline auto make_vector(int nelems, const func& f) -> vector<decltype(f(0))> { auto v = vector<decltype(f(0))>(nelems); for(auto i = 0; i < nelems; i ++) v[i] = f(i); return v; }
template<typename T, typename func>
inline auto make_vector(const vector<T>& vv, const func& f) -> vector<decltype(f(vv[0]))> { auto v = vector<decltype(f(vv[0]))>(vv.size()); for(auto i = 0; i < vv.size(); i ++) v[i] = f(vv[i]); return v; }

// python-like string manipulation
#include "pystring.h"
using namespace pystring;
inline vector<string> splitlines(const string& str, bool keepends=false) { auto result = vector<string>(); splitlines(str, result, keepends); return result; }
inline vector<string> split(const string& str, const string& sep="", int maxsplit=-1) { auto result = vector<string>(); split(str, result, sep, maxsplit); return result; }
// inline string lower(const string& str) { auto ret = str; for(auto i = 0 ; i < str.length(); i ++) ret[i] =tolower(str[i]); return ret; }

// load a text file into a buffer
inline string load_text_file(const char* filename) {
    auto text = string("");
    auto f = fopen(filename,"r");
    error_if_not(f, "could not open file: %s\n", filename);
    char line[4096];
    while (fgets(line, 4096, f)) text += line;
    fclose(f);
    return text;
}

// load a text file into a buffer
inline string load_text_file(const string& filename) { return load_text_file(filename.c_str()); }
inline void save_text_file(const string& filename, const string& str) {
    auto f = fopen(filename.c_str(), "w");
    error_if_not(f, "could not open file: %s\n", filename.c_str());
    fwrite(str.c_str(), sizeof(char), str.length(), f);
    fclose(f);
}

// simple path manipulation (these are not robust, just easy to use)
inline string path_basename(const string& path) { auto pos = path.rfind("/"); if(pos == string::npos) return path; else return path.substr(pos+1); }
inline string path_dirname(const string& path) { auto pos = path.rfind("/"); if(pos == string::npos) return ""; else return path.substr(0,pos+1); }
inline pair<string,string> path_split(const string& path) { auto pos = path.rfind("/"); if(pos == string::npos) return {"",path}; else return {path.substr(0,pos+1),path.substr(pos+1)}; }
inline pair<string,string> path_splitext(const string& path) { auto pos = path.rfind("."); if(pos == string::npos) return {path,""}; else return {path.substr(0,pos),path.substr(pos+1)}; }

// utility to get a file modification time
#include "sys/stat.h"
inline time_t path_modification_time(const string& filename) {
    struct stat stat_buf;
    stat(filename.c_str(), &stat_buf);
    return stat_buf.st_mtime;
}

// base64 encode (http://en.wikibooks.org/wiki/Algorithm_Implementation/Miscellaneous/Base64)
inline string base64_encode(const std::vector<unsigned char>& inputBuffer) {
    const static char encodeLookup[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const static char padCharacter = '=';
    string encodedString;
    encodedString.reserve(((inputBuffer.size()/3) + (inputBuffer.size() % 3 > 0)) * 4);
    long temp;
    std::vector<unsigned char>::const_iterator cursor = inputBuffer.begin();
    for(size_t idx = 0; idx < inputBuffer.size()/3; idx++) {
        temp  = (*cursor++) << 16; //Convert to big endian
        temp += (*cursor++) << 8;
        temp += (*cursor++);
        encodedString.append(1,encodeLookup[(temp & 0x00FC0000) >> 18]);
        encodedString.append(1,encodeLookup[(temp & 0x0003F000) >> 12]);
        encodedString.append(1,encodeLookup[(temp & 0x00000FC0) >> 6 ]);
        encodedString.append(1,encodeLookup[(temp & 0x0000003F)      ]);
    }
    switch(inputBuffer.size() % 3) {
        case 1:
            temp  = (*cursor++) << 16; //Convert to big endian
            encodedString.append(1,encodeLookup[(temp & 0x00FC0000) >> 18]);
            encodedString.append(1,encodeLookup[(temp & 0x0003F000) >> 12]);
            encodedString.append(2,padCharacter);
            break;
        case 2:
            temp  = (*cursor++) << 16; //Convert to big endian
            temp += (*cursor++) << 8;
            encodedString.append(1,encodeLookup[(temp & 0x00FC0000) >> 18]);
            encodedString.append(1,encodeLookup[(temp & 0x0003F000) >> 12]);
            encodedString.append(1,encodeLookup[(temp & 0x00000FC0) >> 6 ]);
            encodedString.append(1,padCharacter);
            break;
    }
    return encodedString;
}

// base64 decode (http://en.wikibooks.org/wiki/Algorithm_Implementation/Miscellaneous/Base64)
inline vector<unsigned char> base64_decode(const string& input) {
    const static char padCharacter = '=';
    if (input.length() % 4) //Sanity check
        throw std::runtime_error("Non-Valid base64!");
    size_t padding = 0;
    if (input.length()) {
        if (input[input.length()-1] == padCharacter)
            padding++;
        if (input[input.length()-2] == padCharacter)
            padding++;
    }
    //Setup a vector to hold the result
    std::vector<unsigned char> decodedBytes;
    decodedBytes.reserve(((input.length()/4)*3) - padding);
    long temp=0; //Holds decoded quanta
    std::basic_string<char>::const_iterator cursor = input.begin();
    while (cursor < input.end()) {
        for (size_t quantumPosition = 0; quantumPosition < 4; quantumPosition++) {
            temp <<= 6;
            if       (*cursor >= 0x41 && *cursor <= 0x5A) // This area will need tweaking if
                temp |= *cursor - 0x41;		              // you are using an alternate alphabet
            else if  (*cursor >= 0x61 && *cursor <= 0x7A)
                temp |= *cursor - 0x47;
            else if  (*cursor >= 0x30 && *cursor <= 0x39)
                temp |= *cursor + 0x04;
            else if  (*cursor == 0x2B)
                temp |= 0x3E; //change to 0x2D for URL alphabet
            else if  (*cursor == 0x2F)
                temp |= 0x3F; //change to 0x5F for URL alphabet
            else if  (*cursor == padCharacter) { //pad
                switch( input.end() - cursor ) {
                    case 1: //One pad character
                        decodedBytes.push_back((temp >> 16) & 0x000000FF);
                        decodedBytes.push_back((temp >> 8 ) & 0x000000FF);
                        return decodedBytes;
                    case 2: //Two pad characters
                        decodedBytes.push_back((temp >> 10) & 0x000000FF);
                        return decodedBytes;
                    default:
                        throw std::runtime_error("Invalid Padding in Base 64!");
                }
            }  else
                throw std::runtime_error("Non-Valid Character in Base 64!");
            cursor++;
        }
        decodedBytes.push_back((temp >> 16) & 0x000000FF);
        decodedBytes.push_back((temp >> 8 ) & 0x000000FF);
        decodedBytes.push_back((temp      ) & 0x000000FF);
    }
    return decodedBytes;
}

#endif
