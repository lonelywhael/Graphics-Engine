#include "io/parser.hpp"

// The escape char is used to signal that something that may be parsed as a flag is not actually a flag. For example if " is a flag
// and enable_escape_char = true, then \" will not be interpreted as a flag.
const char ESCAPE_CHAR = '\\';

// --Some "private" function headers. The first group handles parameters and the second group handles errors--

// Function that checks if a given flag is preceded by an escape char. Returns flase if the flag is a true flag, returns true
// if it is suppressed by the escape char.
bool p_checkIfSuppressed(std::string string, size_t p, bool reverse);
void p_handleInclusion(const p_param parameters, size_t& o, size_t& c, const size_t o_len, const size_t c_len);
void p_handleInclusion(const p_param parameters, size_t& o, size_t& c, const size_t len);
void p_handleInclusion(const p_param parameters, size_t& o, size_t& c);
// This function adjusts a value jump_dist by the lengths of flags depending on include parameters and is intended for the following
// situation: consider a scenario in which we have asked for a flagged substring and need to move the parser past that substring to get
// to the next one. If the flags were included in the substring, we can just take the substring length and use that to move the parser.
// On the other hand, if flags were not included, we need to increase the length of the jump by the length of the unincluded flags.
void p_handleParseJump(const p_param parameters, size_t& jump_dist, const size_t o_len, const size_t c_len);
void p_handleParseJump(const p_param parameters, size_t& jump_dist, const size_t len);
void p_handleParseJump(const p_param parameters, size_t& jump_dist);

// This function is called on the result of a search for a flag that should be present. Will throw an error is the flag is not found.
bool e_missingFlag(size_t p);
// This function is called to throw an extraneous flag error.
void e_extraneousFlag(char c_flag);
void e_extraneousFlag(std::string c_flag);
// This function is called with the result of a search for a closing flag. Will throw an error if given the incorrect closing flag.
bool e_extraneousFlag(char c_top, char c_flag, char _flag);
bool e_extraneousFlag(std::string c_top, std::string c_flag, std::string _flag);


// --Function definitions--

bool p_hasKey(std::string string, char key) { return (string.find(key) != std::string::npos); }
bool p_hasKey(std::string string, std::string key) { return (string.find(key) != std::string::npos); }

// TODO - rethink all this
std::string p_getKeyedSubstr(std::string string, char flag, char key) {
    if (!p_hasKey(string, key)) return "";
    size_t start = string.find(key) + 1, end = string.find(flag, start);
    return string.substr(start, end - start);
}
std::string p_getKeyedSubstr(std::string string, char flag, std::string key) {
    if (!p_hasKey(string, key)) return "";
    size_t start = string.find(key) + key.length(), end = string.find(flag, start);
    return string.substr(start, end - start);
}
std::string p_getKeyedSubstr(std::string string, std::string flag, std::string key) {
    if (!p_hasKey(string, key)) return "";
    size_t start = string.find(key) + key.length(), end = string.find(flag, start);
    return string.substr(start, end - start);
}


size_t p_getNext(std::string string, char flag, p_param parameters) {
    size_t p = string.find(flag);
    // if the escape char is enabled, need to check all flags to make sure they aren't suppressed, if so skip to next
    while ((parameters & P_ENABLE_ESCAPE_CHAR) && p_checkIfSuppressed(string, p, false)) p = string.find(flag, p + 1);
    return p;
}
size_t p_getNext(std::string string, size_t pos, char flag, p_param parameters) { 
    // can't call substr if pos is npos, so just set p to npos
    size_t p = (pos == std::string::npos) ? pos : p_getNext(string.substr(pos), flag, parameters);
    // if we get npos, then we don't want to modify this value, otherwise we should account for the starting position of the substr
    return (p == std::string::npos) ? p : p + pos;
}
size_t p_getNext(std::string string, std::string flag, p_param parameters) {
    // as above
    size_t p = string.find(flag);
    while ((parameters & P_ENABLE_ESCAPE_CHAR) && p_checkIfSuppressed(string, p, false)) p = string.find(flag, p + 1);
    return p;
}
size_t p_getNext(std::string string, size_t pos, std::string flag, p_param parameters) { 
    // as above
    size_t p = (pos == std::string::npos) ? pos : p_getNext(string.substr(pos), flag, parameters);
    return (p == std::string::npos) ? p : p + pos;
}
char p_getFirstOf(std::string string, std::initializer_list<char> flags, p_param parameters) {
    size_t p = std::string::npos, temp;
    // compare locations of all flags and minimize p
    for (char flag : flags) p = ((temp = p_getNext(string, flag, parameters)) < p) ? temp : p;
    // return character at p (as long as at least one flag was found)
    return (p = std::string::npos) ? '\0' : string[p];
}
char p_getFirstOf(std::string string, size_t pos, std::initializer_list<char> flags, p_param parameters)
    { return pos = std::string::npos ? pos : p_getFirstOf(string.substr(pos), flags, parameters); }
std::string p_getFirstOf(std::string string, std::initializer_list<std::string> flags, p_param parameters) {
    size_t p = std::string::npos, temp;
    // set default return value to empty string (will return if no flags are found0)
    std::string _flag = "";
    // compare locations of all flags and minimize p, keeping track of which flag was minimized
    for (std::string flag : flags) if ((temp = p_getNext(string, flag, parameters)) < p) { p = temp; _flag = flag; }
    return _flag;
}
std::string p_getFirstOf(std::string string, size_t pos, std::initializer_list<std::string> flags, p_param parameters)
    { return (pos == std::string::npos) ? "" : p_getFirstOf(string.substr(pos), flags, parameters); }
size_t p_getFirstIndexOf(std::string string, std::initializer_list<char> flags, p_param parameters) {
    size_t p = std::string::npos, temp;
    // search for index of each flag and return the min 
    for (char flag : flags) p = ((temp = p_getNext(string, flag, parameters)) < p) ? temp : p;
    return p;
}
size_t p_getFirstIndexOf(std::string string, size_t pos, std::initializer_list<char> flags, p_param parameters) { 
    size_t p = (pos == std::string::npos) ? pos : p_getFirstIndexOf(string.substr(pos), flags, parameters);
    // if we get npos, then we don't want to modify this value, otherwise we should account for the starting position of the substr
    return (p == std::string::npos) ? p : p + pos; 
}
extern size_t p_getFirstIndexOf(std::string string, std::initializer_list<std::string> flags, p_param parameters) {
    size_t p = std::string::npos, temp;
    // as above
    for (std::string flag : flags) p = ((temp = p_getNext(string, flag, parameters)) < p) ? temp : p;
    return p;
}
size_t p_getFirstIndexOf(std::string string, size_t pos, std::initializer_list<std::string> flags, p_param parameters) { 
    size_t p = (pos == std::string::npos) ? pos : p_getFirstIndexOf(string.substr(pos), flags, parameters);
    // as above
    return (p == std::string::npos) ? p : p + pos; 
}




size_t p_getLast(std::string string, std::string flag, p_param parameters) {
    // reverse the string and flag
    std::string r_string = string, r_flag = flag;
    std::reverse(r_string.begin(), r_string.end());
    std::reverse(r_flag.begin(), r_flag.end());

    // find the first occurrance 
    size_t p = r_string.find(r_flag);

    // check for suppression after of the flag instead of before
    while ((parameters & P_ENABLE_ESCAPE_CHAR) && p_checkIfSuppressed(r_string, p + flag.length() - 1, true)) p = r_string.find(r_flag);
    // if p returns as npos, then we shouldn't change it, otherwise need to convert p to an index on the original string
    return (p == std::string::npos) ? p : string.length() - p - flag.length();
}




std::string p_getNextSubstr(std::string string, char breaker, p_param parameters) {
    // can include or exclude the breaker
    size_t c = p_getNext(string, breaker, parameters) + (parameters & P_INCLUDE_CLOSE) ? 1 : 0;
    return string.substr(0, c); 
}
std::string p_getNextSubstr(std::string string, size_t pos, char breaker, p_param parameters) 
    { return (pos == std::string::npos) ? "" : string.substr(pos, p_getNext(string, pos, breaker, parameters) - pos); }
std::string p_getNextSubstr(std::string string, std::string breaker, p_param parameters) {
    // as above
    size_t c = p_getNext(string, breaker, parameters) + (parameters & P_INCLUDE_CLOSE) ? breaker.length() : 0;
    return string.substr(0, c); 
}
std::string p_getNextSubstr(std::string string, size_t pos, std::string breaker, p_param parameters) 
    { return (pos == std::string::npos) ? "" : string.substr(pos, p_getNext(string, pos, breaker, parameters) - pos); }



std::string p_getFlaggedSubstr(std::string string, char flag, p_param parameters) {
    // get the first occurance of the flag
    size_t start = p_getNext(string, flag, parameters);
    // if the flag doesn't occur, that is okay, just return nothing as nothing was found
    if (start == std::string::npos) return "";
    // find the next flag after the first
    size_t end = p_getNext(string, start + 1, flag, parameters);
    // check if a closing flag was found
    if (e_missingFlag(end)) return "";
    // return the substr bounded by the opening and closing flag, handling inclusing according to parameters
    p_handleInclusion(parameters, start, end);
    return string.substr(start, end - start);
}
std::string p_getFlaggedSubstr(std::string string, size_t pos, char flag, p_param parameters) 
    { return (pos == std::string::npos) ? "" : p_getFlaggedSubstr(string.substr(pos), flag, parameters); }
std::string p_getFlaggedSubstr(std::string string, std::string flag, p_param parameters) {
    // as above
    size_t start = p_getNext(string, flag, parameters);
    if (start == std::string::npos) return "";
    size_t end = p_getNext(string, start + 1, flag, parameters);
    if (e_missingFlag(end)) return "";
    p_handleInclusion(parameters, start, end, flag.length());
    return string.substr(start, end - start);
}
std::string p_getFlaggedSubstr(std::string string, size_t pos, std::string flag, p_param parameters) 
    { return (pos == std::string::npos) ? "" : p_getFlaggedSubstr(string.substr(pos), flag, parameters); }
std::string p_getFlaggedSubstr(std::string string, std::initializer_list<std::tuple<char, char>> flags, p_param parameters) {
    bool iec = parameters & P_IGNORE_EXTRANEOUS_CLOSE;
    // create a stack of open flags that will keep track of nestings as well as what symbol is needed to close the current nesting
    std::stack<std::tuple<char, char>> flag_stack;
    // find the first open flag
    size_t start = std::string::npos, temp;
    char _o_flag, _c_flag;  // temporary values that allow us to keep the flag pair that is minimized for later use
    for (auto [o_flag, c_flag] : flags) 
        if ((temp = p_getNext(string, o_flag, parameters)) < start) { start = temp; _o_flag = o_flag; _c_flag = c_flag; }
    // if there are no opening flags, this is not necessarily an error but the function will return empty
    if (start == std::string::npos) return "";
    // otherwise, add the first open flag and its closing pair to the stack
    flag_stack.push( { _o_flag, _c_flag } );

    // look for the next flag. if it is a new open flag, push to stack. if it closes the current open flag, pop. repeat until stack empty
    size_t p = start;
    while (flag_stack.size() > 0) {
        // save top of stack for use in comparisons
        auto [o_top, c_top] = flag_stack.top();
        // minimize _p to find next flag
        // if ignoring extraneous close markers, we can shorten the amount of computation by only searching for the close flag to the 
        // current open flag. Otherwise check for every possible close flag.
        size_t _p = iec ? p_getNext(string, p + 1, c_top, parameters) : std::string::npos;
        for (auto [o_flag, c_flag] : flags) {
            temp = iec ?
                p_getNext(string, p + 1, o_flag, parameters):
                std::min(p_getNext(string, p + 1, o_flag, parameters), p_getNext(string, p + 1, c_flag, parameters));
            if (temp < _p) { _p = temp; _o_flag = o_flag; _c_flag = c_flag; }
        }
        p = _p;
        // handle errors, i.e. no flags were found or a closing flag was found that doesn't close the current open flag
        if (e_missingFlag(p) || (!iec && e_extraneousFlag(c_top, _c_flag, string[p]))) return "";
        // otherwise, check if the next flag closes the previous or else add another pair to the stack
        (string[p] == c_top) ? flag_stack.pop() : flag_stack.push( { _o_flag, _c_flag } );
    }

    // return everything between the first opening flag and the flag that closes it
    p_handleInclusion(parameters, start, p);
    return string.substr(start, p - start);
}
std::string p_getFlaggedSubstr(std::string string, size_t pos, std::initializer_list<std::tuple<char, char>> flags, 
                               p_param parameters) 
    { return (pos == std::string::npos) ? "" : p_getFlaggedSubstr(string.substr(pos), flags, parameters); }
std::string p_getFlaggedSubstr(std::string string, std::initializer_list<std::tuple<std::string, std::string>> flags, 
                               p_param parameters) {
    bool iec = parameters & P_IGNORE_EXTRANEOUS_CLOSE;
    // create a stack of open flags that will keep track of nestings as well as what symbol is needed to close the current nesting
    std::stack<std::tuple<std::string, std::string>> flag_stack;
    // find the first open flag
    size_t start = std::string::npos, temp;
    std::string _o_flag, _c_flag;  // temporary values that allow us to keep the flag pair that is minimized for later use
    for (auto [o_flag, c_flag] : flags) 
        if ((temp = p_getNext(string, o_flag, parameters)) < start) { start = temp; _o_flag = o_flag; _c_flag = c_flag; }
    // if there are no opening flags, this is not necessarily an error but the function will return empty
    if (start == std::string::npos) return "";
    // otherwise, add the first open flag and its closing pair to the stack
    flag_stack.push( { _o_flag, _c_flag } );
    size_t o_len = _o_flag.length(), c_len = _c_flag.length();

    // look for the next flag. if it is a new open flag, push to stack. if it closes the current open flag, pop. repeat until stack empty
    size_t p = start + 1;
    while (flag_stack.size() > 0) {
        // save top of stack for use in comparisons
        std::string flag = "";
        auto [o_top, c_top] = flag_stack.top();
        // minimize _p to find next flag
        size_t _p = iec ? p_getNext(string, p + 1, c_top, parameters) : std::string::npos;
        for (auto [o_flag, c_flag] : flags) {
            if ((temp = p_getNext(string, p + 1, o_flag, parameters)) < _p) 
                { _p = temp; _o_flag = o_flag; _c_flag = c_flag; flag = o_flag; }
            if (!iec && (temp = p_getNext(string, p + 1, c_flag, parameters)) < _p) 
                { _p = temp; _o_flag = o_flag; _c_flag = c_flag; flag = c_flag; }
        }
        p = _p;
        // handle errors, i.e. no flags were found or a closing flag was found that doesn't close the current open flag
        if (e_missingFlag(p) || (!iec && e_extraneousFlag(c_top, _c_flag, flag))) return "";
        // otherwise, check if the next flag closes the previous or else add another pair to the stack
        (flag == c_top) ? flag_stack.pop() : flag_stack.push( { _o_flag, _c_flag } );
    }

    p_handleInclusion(parameters, start, p, o_len, c_len);
    // return everything between the first opening flag and the flag that closes it
    return string.substr(start, p - start);
}
std::string p_getFlaggedSubstr(std::string string, size_t pos, std::initializer_list<std::tuple<std::string, std::string>> flags, 
                               p_param parameters) 
    { return (pos == std::string::npos) ? "" : p_getFlaggedSubstr(string.substr(pos), flags, parameters); }



std::string p_removeFlagged(std::string string, std::string o_flag, std::string c_flag, p_param parameters) {
    bool iec = parameters & P_IGNORE_EXTRANEOUS_CLOSE;
    // beg is from beginning to first open flag, end is last close flag to end, mid is everything else
    std::string beg, mid, end;
    // find first and last of both open and close flags
    size_t _o = p_getNext(string, o_flag, parameters), 
           c = p_getNext(string, iec ? _o : 0, c_flag, parameters),
           o = p_getLast(string, o_flag, parameters), 
           _c = iec ? p_getNext(string, o, c_flag, parameters) : p_getLast(string, c_flag, parameters);
    // handle errors, if close comes before first open, open comes after last close, open without close or close without open
    if (!iec) {
        if (c < _o) { e_extraneousFlag(c_flag); return string; }
        if (o > _c) { e_extraneousFlag(o_flag); return string; }
    }
    if (o == std::string::npos) return string; // this is not an error, just means there were no flags
    if (e_missingFlag(_c)) return string;

    // adjust ranges based on whether flags are to be removed or not
    beg = string.substr(0, (parameters & P_INCLUDE_OPEN) ? _o : _o + o_flag.length()); // beg is everything up to the first open flag
    end = string.substr((parameters & P_INCLUDE_CLOSE) ? _c + c_flag.length() : _c);    // end is everything after the last close flag
    // parse everything between first open and last close
    size_t p = c; // skip to the next close flag
    while (p < _c && p_getNext(string, p, o_flag, parameters) != std::string::npos) { 
        // ask for flagged substrings by treating close flags as open and open as close
        // create a new parameter set that switches the values of include close and include open
        p_param _parameters = parameters & (~P_INCLUSIVE) +                           // flip both inclusive bits off
                              ((parameters & P_INCLUDE_OPEN) ? 0 : P_INCLUDE_CLOSE) + // flip close on if open was on
                              ((parameters & P_INCLUDE_CLOSE) ? 0 : P_INCLUDE_OPEN);  // flip open on if close was on
        // if we are ignoring extraneous closing flags, then just treat o_flag as a breaker and get next substr
        std::string temp = iec ? p_getNextSubstr(string, p + ((_parameters & P_INCLUDE_OPEN) ? 0 : c_flag.length()), o_flag, _parameters) :
                                 p_getFlaggedSubstr(string, p, { { c_flag, o_flag } }, _parameters);
        // append to mid
        mid += temp;
        // p moves to the first close flag found after the last open flag
        size_t parse_jump = p + temp.length();
        p_handleParseJump(_parameters, parse_jump, o_flag.length(), c_flag.length());
        p = p_getNext(string, parse_jump, c_flag, _parameters);
    }
    return beg + mid + end;
}
std::string p_removeAll(std::string string, std::initializer_list<char> symbols) { 
    std::string _string = string;
    // TODO - don't really understand how iterators work so I could not tell you why this works. 
    for (char symbol : symbols)
        _string.erase(std::remove(_string.begin(), _string.end(), symbol), _string.end());
    return _string;
}
std::string p_removeAll(std::string string, size_t pos, std::initializer_list<char> symbols)
    { return (pos == std::string::npos) ? "" : p_removeAll(string.substr(pos), symbols); }
std::string p_removeAllExceptFlagged(std::string string, std::initializer_list<char> symbols, char flag, p_param parameters) {
    // since this function operates on characters and does not return substrings, the inclusion parameter doesn't do anything
    parameters |= P_INCLUSIVE; // always need to include flags
    
    // set p as the beginning of the substr and _p as the next occurance of the opening flag. p starts at beginning and _p is first flag
    size_t p = 0, _p = p_getNext(string, flag, parameters);
    std::string _string = "";
    // remove symbols from substr between p and _p and at that to output string. then append the flagged string as is
    while (_p != std::string::npos) {
        std::string temp;
        // remove symbols from unflagged region
        _string += p_removeAll(string.substr(p, _p - p), symbols);              
        // add flagged region as is
        _string += (temp = p_getFlaggedSubstr(string, _p, flag, parameters));    

        p = _p + temp.length();
        _p = p_getNext(string, p, flag, parameters); // find subsequent next flag
    }
    // remove symbols from anything that occurs after the final flagged region and append that as well
    _string += p_removeAll(string, p, symbols);

    return _string;
}
std::string p_removeAllExceptFlagged(std::string string, size_t pos, std::initializer_list<char> symbols, char flag, p_param parameters)
    { return (pos == std::string::npos) ? "" : p_removeAllExceptFlagged(string.substr(pos), symbols, flag, parameters); }


std::string p_getSnippet(std::string string, size_t pos) {
    // return a short snippet of text around a given index
    if (pos == std::string::npos) return "";
    const size_t LEN = 5;
    // check to make sure there is actually a buffer around the index otherwise just cut the string off at the beginning or end
    size_t beg = (pos < LEN) ? 0 : pos - LEN, end = (string.length() - pos < LEN) ? string.length() - 1 : pos + LEN;
    return ((beg == 0) ? "" : "...") + string.substr(beg, end - beg) + ((end == string.length() - 1) ? "" : "...");
}


bool p_checkIfSuppressed(std::string string, size_t p, bool reverse) {
    return (p == std::string::npos) ? false : ( // if p is npos just return false as there is no possibility of there being an escape flag
           reverse ? 
            (p == string.length() - 1) ? false : string[p + 1] == ESCAPE_CHAR : // in the reverse case, just character ahead of p
            (p == 0) ? false : string[p - 1] == ESCAPE_CHAR ); // in the normal case, check the character behind p
}
void p_handleInclusion(const p_param parameters, size_t& o, size_t& c, const size_t o_len, const size_t c_len) {
    // by default, the open flag will be included and the close flag will be excluded
    o += (parameters & P_INCLUDE_OPEN) ? 0 : o_len;  // exclude the open flag by adjusting the start of the substr forward
    c += (parameters & P_INCLUDE_CLOSE) ? c_len : 0; // include the close flag by adjusting the end of the substr forward
}
void p_handleInclusion(const p_param parameters, size_t& o, size_t& c, const size_t len) 
    { p_handleInclusion(parameters, o, c, len, len); }
void p_handleInclusion(const p_param parameters, size_t& o, size_t& c) 
    { p_handleInclusion(parameters, o, c, 1, 1); }
void p_handleParseJump(const p_param parameters, size_t& jump_dist, const size_t o_len, const size_t c_len) 
    { jump_dist += ((parameters & P_INCLUDE_OPEN) ? 0 : o_len) + ((parameters & P_INCLUDE_CLOSE) ? 0 : c_len); }
void p_handleParseJump(const p_param parameters, size_t& jump_dist, const size_t len)
    { p_handleParseJump(parameters, jump_dist, len, len); }
void p_handleParseJump(const p_param parameters, size_t& jump_dist)
    { p_handleParseJump(parameters, jump_dist, 1, 1); }


bool e_missingFlag(size_t p) { 
    if (p == std::string::npos) {
        std::cout << "ERROR::PARSE::MISSING_CLOSING_FLAG: Closing flags do not match opening flags." << std::endl;
        return true;
    }
    return false;
}
void e_extraneousFlag(char c_flag) {
    std::cout << "ERROR::PARSE::EXTRANEOUS_CLOSING_FLAG: Closing flag \'" << c_flag << "\' without matching opening flag." << std::endl;
}
void e_extraneousFlag(std::string c_flag) {
    std::cout << "ERROR::PARSE::EXTRANEOUS_CLOSING_FLAG: Closing flag \'" << c_flag << "\' without matching opening flag." << std::endl;
}
bool e_extraneousFlag(char c_top, char c_flag, char _flag) {
    if (c_flag != c_top && c_flag == _flag) {
        e_extraneousFlag(c_flag);
        return true;
    }
    return false;
}
bool e_extraneousFlag(std::string c_top, std::string c_flag, std::string _flag) {
    if (c_flag != c_top && c_flag == _flag) {
        e_extraneousFlag(c_flag);
        return true;
    }
    return false;
}