#ifndef _JSON_H_
#define _JSON_H_

#include "common.h"

struct json {
    using array = vector<json>;
    using object = map<string,json>;
    enum type_t { null_t, bool_t, number_t, string_t, array_t, object_t };
    type_t          type = null_t;
    union { bool bool_val; double number_val; string* string_val; array* array_val; object* object_val; };
    
    json() : type(null_t) { }
    
    json(bool v) : type(bool_t), bool_val(v) { }
    json(double v) : type(number_t), number_val(v) { }
    json(const string& v) : type(string_t), string_val(new string(v)) { }
    json(const array& v) : type(array_t), array_val(new array(v)) { }
    json(const object& v) : type(object_t), object_val(new object(v)) { }
    template <typename T> json(T*) = delete;
    
    json(int v) : type(number_t), number_val(v) { }
    
    json(const json& js) : json() { _copy(js); }
    json& operator=(const json& js) { if(this != &js) _copy(js); return *this; }
    
    ~json() { _clear(); }
    
    bool is_null() const { return type == null_t; }
    bool is_bool() const { return type == bool_t; }
    bool is_number() const { return type == number_t; }
    bool is_string() const { return type == string_t; }
    bool is_array() const { return type == array_t; }
    bool is_object() const { return type == object_t; }
    
    bool& bool_ref() { error_if_not(is_bool(), "bad type"); return bool_val; }
    double& number_ref() { error_if_not(is_number(), "bad type"); return number_val; }
    string& string_ref() { error_if_not(is_string(), "bad type"); return *string_val; }
    array& array_ref() { error_if_not(is_array(), "bad type"); return *array_val; }
    object& object_ref() { error_if_not(is_object(), "bad type"); return *object_val; }
    
    bool bool_value() const { error_if_not(is_bool(), "bad type"); return bool_val; }
    int int_value() const { error_if_not(is_number(), "bad type"); return (int)number_val; }
    double number_value() const { error_if_not(is_number(), "bad type"); return number_val; }
    const string& string_value() const { error_if_not(is_string(), "bad type"); return *string_val; }
    const array& array_value() const { error_if_not(is_array(), "bad type"); return *array_val; }
    const object& object_value() const { error_if_not(is_object(), "bad type"); return *object_val; }
    
    const array& array_items() const { error_if_not(is_array(), "bad type"); return *array_val; }
    const object& object_items() const { error_if_not(is_object(), "bad type"); return *object_val; }
    
    int size() const { if(is_array()) return array_val->size(); if(is_object()) return object_val->size(); return 0; }
    bool has_member(const string& key) const { error_if_not(is_object(), "bad type"); return object_val->find(key) != object_val->end(); }
    json& operator[](int i) { error_if_not(is_array(), "bad type"); return array_val->at(i); }
    json& operator[](const string& key) { error_if_not(is_object(), "bad type"); return object_val->at(key); }
    const json& operator[](int i) const { error_if_not(is_array(), "bad type"); return array_val->at(i); }
    const json& operator[](const string& key) const { error_if_not(is_object(), "bad type"); return object_val->at(key); }
    
    static json parse(const string& str);
    static json load(const string& filename) { return parse(load_text_file(filename)); }
    string dump() const;
    string dump_pretty(int level) const;
    void save(const string& filename, bool pretty = true) {
        auto str = (pretty) ? dump_pretty(0) : dump();
        save_text_file(filename, str);
    }

    void _clear() { if(type == string_t) delete string_val; if(type == array_t) delete array_val; if(type == object_t) delete object_val; type = null_t;  }
    void _copy(const json& js) {
        _clear();
        type = js.type;
        switch(type) {
            case null_t: break;
            case bool_t: bool_val = js.bool_val; break;
            case number_t: number_val = js.number_val; break;
            case string_t: string_val = new string(*js.string_val); break;
            case array_t: array_val = new array(*js.array_val); break;
            case object_t: object_val = new object(*js.object_val); break;
            default: error("should not have gotten here");
        }
    }

#ifndef _USE_JSON11_
    static string _dump_escape_string(const string& str);
    static void _write(FILE* f, const string& str) { fwrite(str.c_str(), str.size(), 1, f); }
#else
#endif
};

#ifndef _USE_JSON11_
// from json11
struct _parser {
    const string &str;
    size_t i;
    string &err;
    bool failed;
    int max_depth = 200;

	_parser(const string& str_, size_t i_, string &err_, bool failed_) : str(str_), i(i_), err(err_), failed(failed_) {}
    
    static string esc(char c) {
        char buf[12];
        if (static_cast<uint8_t>(c) >= 0x20 && static_cast<uint8_t>(c) <= 0x7f) {
            snprintf(buf, sizeof buf, "'%c' (%d)", c, c);
        } else {
            snprintf(buf, sizeof buf, "(%d)", c);
        }
        return string(buf);
    }
    
    static bool in_range(long x, long lower, long upper) {
        return (x >= lower && x <= upper);
    }
    
    inline json fail(string &&msg) { return fail(move(msg), json()); }
    
    template <typename T>
    inline T fail(string &&msg, const T err_ret) {
        if (!failed) err = std::move(msg);
        failed = true;
        return err_ret;
    }
    
    inline void consume_whitespace() {
        while (str[i] == ' ' || str[i] == '\r' || str[i] == '\n' || str[i] == '\t') i++;
    }
    
    // Return the next non-whitespace character. If the end of the input is reached, flag an error and return 0.
    inline char get_next_token() {
        consume_whitespace();
        if (i == str.size()) return fail("unexpected end of input", 0);
        return str[i++];
    }
    
    // encode_utf8(pt, out) and add it to out
    inline void encode_utf8(long pt, string & out) {
        if (pt < 0) return;
        if (pt < 0x80) {
            out += pt;
        } else if (pt < 0x800) {
            out += (pt >> 6) | 0xC0;
            out += (pt & 0x3F) | 0x80;
        } else if (pt < 0x10000) {
            out += (pt >> 12) | 0xE0;
            out += ((pt >> 6) & 0x3F) | 0x80;
            out += (pt & 0x3F) | 0x80;
        } else {
            out += (pt >> 18) | 0xF0;
            out += ((pt >> 12) & 0x3F) | 0x80;
            out += ((pt >> 6) & 0x3F) | 0x80;
            out += (pt & 0x3F) | 0x80;
        }
    }
    
    string parse_string() {
        string out;
        long last_escaped_codepoint = -1;
        while (true) {
            if (i == str.size()) return fail("unexpected end of input in string", "");
            char ch = str[i++];
            if (ch == '"') {
                encode_utf8(last_escaped_codepoint, out);
                return out;
            }
            if (in_range(ch, 0, 0x1f)) return fail("unescaped " + esc(ch) + " in string", "");
            // The usual case: non-escaped characters
            if (ch != '\\') {
                encode_utf8(last_escaped_codepoint, out);
                last_escaped_codepoint = -1;
                out += ch;
                continue;
            }
            // Handle escapes
            if (i == str.size()) return fail("unexpected end of input in string", "");
            ch = str[i++];
            if (ch == 'u') {
                // Extract 4-byte escape sequence
                string esc = str.substr(i, 4);
                for (int j = 0; j < 4; j++) {
                    if (!in_range(esc[j], 'a', 'f') && !in_range(esc[j], 'A', 'F')
                        && !in_range(esc[j], '0', '9'))
                        return fail("bad \\u escape: " + esc, "");
                }
                long codepoint = strtol(esc.data(), nullptr, 16);
                // JSON specifies that characters outside the BMP shall be encoded as a pair
                // of 4-hex-digit \u escapes encoding their surrogate pair components. Check
                // whether we're in the middle of such a beast: the previous codepoint was an
                // escaped lead (high) surrogate, and this is a trail (low) surrogate.
                if (in_range(last_escaped_codepoint, 0xD800, 0xDBFF)
                    && in_range(codepoint, 0xDC00, 0xDFFF)) {
                    // Reassemble the two surrogate pairs into one astral-plane character, per
                    // the UTF-16 algorithm.
                    encode_utf8((((last_escaped_codepoint - 0xD800) << 10)
                                 | (codepoint - 0xDC00)) + 0x10000, out);
                    last_escaped_codepoint = -1;
                } else {
                    encode_utf8(last_escaped_codepoint, out);
                    last_escaped_codepoint = codepoint;
                }
                
                i += 4;
                continue;
            }
            
            encode_utf8(last_escaped_codepoint, out);
            last_escaped_codepoint = -1;
            
            if (ch == 'b') {
                out += '\b';
            } else if (ch == 'f') {
                out += '\f';
            } else if (ch == 'n') {
                out += '\n';
            } else if (ch == 'r') {
                out += '\r';
            } else if (ch == 't') {
                out += '\t';
            } else if (ch == '"' || ch == '\\' || ch == '/') {
                out += ch;
            } else {
                return fail("invalid escape character " + esc(ch), "");
            }
        }
    }
    
    // parses a double
    json parse_number() {
        size_t start_pos = i;
        
        if (str[i] == '-') i++;
        
        // Integer part
        if (str[i] == '0') {
            i++;
            if (in_range(str[i], '0', '9'))
                return fail("leading 0s not permitted in numbers");
        } else if (in_range(str[i], '1', '9')) {
            i++;
            while (in_range(str[i], '0', '9'))
                i++;
        } else {
            return fail("invalid " + esc(str[i]) + " in number");
        }
        
        if (str[i] != '.' && str[i] != 'e' && str[i] != 'E'
            && (i - start_pos) <= static_cast<size_t>(std::numeric_limits<int>::digits10)) {
            return std::atoi(str.c_str() + start_pos);
        }
        
        // Decimal part
        if (str[i] == '.') {
            i++;
            if (!in_range(str[i], '0', '9'))
                return fail("at least one digit required in fractional part");
            
            while (in_range(str[i], '0', '9'))
                i++;
        }
        
        // Exponent part
        if (str[i] == 'e' || str[i] == 'E') {
            i++;
            
            if (str[i] == '+' || str[i] == '-')
                i++;
            
            if (!in_range(str[i], '0', '9'))
                return fail("at least one digit required in exponent");
            
            while (in_range(str[i], '0', '9'))
                i++;
        }
        
        return std::atof(str.c_str() + start_pos);
    }
    
    // Expect that 'str' starts at the character that was just read. If it does, advance
    // the input and return res. If not, flag an error.
    json _expect(const string &expected, const json& res) {
        assert(i != 0);
        i--;
        if (str.compare(i, expected.length(), expected) == 0) {
            i += expected.length();
            return res;
        } else {
            return fail("parse error: expected " + expected + ", got " + str.substr(i, expected.length()));
        }
    }
    
    // Parse a JSON object.
    json parse_json(int depth) {
        if (depth > max_depth) {
            return fail("exceeded maximum nesting depth");
        }
        
        char ch = get_next_token();
        if (failed) return json();
        
        if (ch == '-' || (ch >= '0' && ch <= '9')) { i--; return parse_number(); }
        
        if (ch == 't') return _expect("true", true);
        
        if (ch == 'f') return _expect("false", false);
        
        if (ch == 'n') return _expect("null", json());
        
        if (ch == '"') return parse_string();
        
        if (ch == '{') {
            map<string, json> data;
            ch = get_next_token();
            if (ch == '}')
                return data;
            
            while (1) {
                if (ch != '"')
                    return fail("expected '\"' in object, got " + esc(ch));
                
                string key = parse_string();
                if (failed) return json();
                
                ch = get_next_token();
                if (ch != ':') return fail("expected ':' in object, got " + esc(ch));
                
                data[std::move(key)] = parse_json(depth + 1);
                if (failed) return json();
                
                ch = get_next_token();
                if (ch == '}') break;
                if (ch != ',') return fail("expected ',' in object, got " + esc(ch));
                
                ch = get_next_token();
            }
            return data;
        }
        
        if (ch == '[') {
            vector<json> data;
            ch = get_next_token();
            if (ch == ']')
                return data;
            
            while (1) {
                i--;
                data.push_back(parse_json(depth + 1));
                if (failed) return json();
                
                ch = get_next_token();
                if (ch == ']') break;
                if (ch != ',') return fail("expected ',' in list, got " + esc(ch));
                
                ch = get_next_token();
                (void)ch;
            }
            return data;
        }
        
        return fail("expected value, got " + esc(ch));
    }
    
    static json parse(const string &in, string &err) {
        _parser parser { in, 0, err, false };
        json result = parser.parse_json(0);
        
        // Check for any trailing garbage
        parser.consume_whitespace();
        if (parser.i != in.size())
            return parser.fail("unexpected trailing " + esc(in[parser.i]));
        
        return result;
    }
    
    // Documented in json11.hpp
    static vector<json> parse_multi(const string &in, string &err) {
        _parser parser { in, 0, err, false };
        
        vector<json> json_vec;
        while (parser.i != in.size() && !parser.failed) {
            json_vec.push_back(parser.parse_json(0));
            // Check for another object
            parser.consume_whitespace();
        }
        return json_vec;
    }
};

inline json json::parse(const string& str) {
    auto err = ""s;
    auto js = _parser::parse(str,err);
    error_if_not(!js.is_null(), "error parsing json: %s", err.c_str());
    return js;
}

// from json11
inline string json::_dump_escape_string(const string& str) {
    auto out = ""s;
    for (size_t i = 0; i < str.length(); i++) {
        const char ch = str[i];
        if (ch == '\\') {
            out += "\\\\";
        } else if (ch == '"') {
            out += "\\\"";
        } else if (ch == '\b') {
            out += "\\b";
        } else if (ch == '\f') {
            out += "\\f";
        } else if (ch == '\n') {
            out += "\\n";
        } else if (ch == '\r') {
            out += "\\r";
        } else if (ch == '\t') {
            out += "\\t";
        } else if (static_cast<uint8_t>(ch) <= 0x1f) {
            char buf[8];
            snprintf(buf, sizeof buf, "\\u%04x", ch);
            out += buf;
        } else if (static_cast<uint8_t>(ch) == 0xe2 && static_cast<uint8_t>(str[i+1]) == 0x80
                   && static_cast<uint8_t>(str[i+2]) == 0xa8) {
            out += "\\u2028";
            i += 2;
        } else if (static_cast<uint8_t>(ch) == 0xe2 && static_cast<uint8_t>(str[i+1]) == 0x80
                   && static_cast<uint8_t>(str[i+2]) == 0xa9) {
            out += "\\u2029";
            i += 2;
        } else {
            out += ch;
        }
    }
    return out;
}

inline string json::dump() const {
    switch(type) {
        case null_t: return "null";
        case bool_t: return (bool_val) ? "true" : "false";
        case number_t: {
            // from json11
            char buf[64];
            snprintf(buf, sizeof(buf), "%.17g", number_val);
            return string(buf);
        }
        case string_t: return "\"" + _dump_escape_string(*string_val) + "\"";
        case array_t: {
            auto ret = "["s;
            auto first = true;
            for(auto&& v : *array_val) {
                if(!first) ret += ","; else first = false;
                ret += v.dump();
            }
            ret += "]";
            return ret;
        } break;
        case object_t: {
            auto ret = "{"s;
            auto first = true;
            for(auto&& v : *object_val) {
                if(!first) ret += ","; else first = false;
                ret += "\"" + _dump_escape_string(v.first) + "\":" + v.second.dump();
            }
            ret += "}";
            return ret;
        } break;
        default: { error("should not have gotten here"); return ""; }
    }
}

inline string json::dump_pretty(int level) const {
    const int max_indent = 6;
    auto _indents = vector<string>{ ""s, "    "s, "        "s, "            "s, "                "s, "                    "s, "                        "s };
    switch(type) {
        case null_t: return dump();
        case bool_t: return dump();
        case number_t: return dump();
        case string_t: return dump();
        case array_t: {
            if(array_val->empty() || std::all_of(array_val->begin(), array_val->end(), [](auto&& v){return v.is_number();})) return dump();
            auto ret = "[\n" + _indents[clamp(level+1,0,max_indent)];
            auto first = true;
            for(auto&& v : *array_val) {
                if(!first) ret += ",\n"+_indents[clamp(level+1,0,max_indent)]; else first = false;
                ret += v.dump_pretty(level+1);
            }
            ret += "\n"+_indents[clamp(level,0,max_indent)]+"]";
            return ret;
        } break;
        case object_t: {
            if(object_val->empty()) return dump();
            auto ret = "{\n" + _indents[clamp(level+1,0,max_indent)];
            auto first = true;
            for(auto&& v : *object_val) {
                if(!first) ret += ",\n"+_indents[clamp(level+1,0,max_indent)]; else first = false;
                ret += "\"" + _dump_escape_string(v.first) + "\": " + v.second.dump_pretty(level+1);
            }
            ret += "\n"+_indents[clamp(level,0,max_indent)]+"}";
            return ret;
        } break;
        default: { error("should not have gotten here"); return ""; }
    }
}

#endif

#ifdef _USE_JSON11_
#include "ext/json11.hpp"    fcfvcfd fg cdymn hn 

static json from_json11(const json11::Json& js) {
    switch (js.type()) {
        case json11::Json::NUL: return json();
        case json11::Json::BOOL: return json(js.bool_value());
        case json11::Json::NUMBER: return json(js.number_value());
        case json11::Json::STRING: return json(js.string_value());
        case json11::Json::ARRAY: {
            json::array ret;
            for (auto&& v : js.array_items()) ret.push_back(from_json11(v));
            return ret;
        } break;
        case json11::Json::OBJECT: {
            json::object ret;
            for (auto&& kv : js.object_items()) ret[kv.first] = from_json11(kv.second);
            return ret;
        } break;
        default: error("should have gotten here");
    }
    return json();
}

json json::parse(const string& str) {
    auto err = ""s;
    auto js = json11::Json::parse(str, err);
    error_if_not(not js.is_null(), "error parsing json: %s", err.c_str());
    return from_json11(js);
}

json11::Json to_json11() const {
    switch(type) {
        case null_t: return json11::Json();
        case bool_t: return json11::Json(bool_val);
        case number_t: return json11::Json(number_val);
        case string_t: return json11::Json(*string_val);
        case array_t: {
            auto js = json11::Json::array();
            for(auto&& v : *array_val) js.push_back(v.to_json11());
            return js;
        } break;
        case object_t: {
            auto js = json11::Json::object();
            for(auto&& kv : *object_val) js[kv.first] = kv.second.to_json11();
            return js;
        } break;
        default: { error("should not have gotten here\n"); return json11::Json(); }
    }
}

string json::_dump() const {
    return to_json11().dump();
}
#endif


#endif
