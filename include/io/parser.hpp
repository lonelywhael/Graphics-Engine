#ifndef PARSER_HPP
#define PARSER_HPP

#include <algorithm>
#include <initializer_list>
#include <iostream>
#include <stack>
#include <string>
#include <tuple>

/* These utility functions are used for parsing text. Some functions simply expand upon the basic functionality of the parsing methods
 * in std::string while others are designed with the goal of parsing specific file formats and code-like structures, especially those
 * that make use of flags. 
 * 
 * Flags:
 * Flags are chars or strings that the parser can look for to designate the region lying in between for a certain treatment. There are
 * two kinds of flags that these methods can handle. First, there are unpaired flags. These are singular char or string symbols that
 * can be searched for (e.g., commas are often treated this way). One way we could use these is to separate substrings from one another. 
 * Another way is to simulate pair behavior by treating every other flag as an opening flag, and the flag that follows it as a closing 
 * flag (e.g., quotation marks). There are also paired flags, which are sets of two flags - one that opens and one that closes (e.g., 
 * brackets). The advantage of paired flags over unpaired flags is that paired flags allow for nesting behavior.
 * 
 * Parameters:
 * There are also some key parameters that can be used to effect parsing behavior. See listed below.
 */

typedef char p_param;
enum parameters : p_param {
    P_DEFAULT = 0b0000,                     // No flags
    P_ENABLE_ESCAPE_CHAR = 0b0001,          // Signals that flags preceded by the escape char '\\' should not be treated as flags
    P_NONINCLUSIVE = 0b0000,                // Signals that a given operation should not be done on the flags, only the text enclosed
    P_INCLUDE_OPEN = 0b0010,                // Signals that a given operation should be performed on the open flag
    P_INCLUDE_CLOSE = 0b0100,               // Signals that a given operation should be performed on the close flag
    P_INCLUSIVE = 0b0110,                   // Signals that a given operation should be performed on all flags
    P_IGNORE_EXTRANEOUS_CLOSE = 0b1000      // Signals that close flags that don't have matching open flags should not throw errors
};

// This function checks whether a string contains a certain key.
extern bool p_hasKey(std::string string, char key);
// This function checks whether a string contains a certain key.
extern bool p_hasKey(std::string string, std::string key);

// This function takes a key char and a flag char a returns the text that lies between the key and the closing flag.
extern std::string p_getKeyedSubstr(std::string string, char flag, char key);
// This function takes a key string and a flag char a returns the text that lies between the key and the closing flag.
extern std::string p_getKeyedSubstr(std::string string, char flag, std::string key);
// This function takes a key string and a flag string a returns the text that lies between the key and the closing flag.
extern std::string p_getKeyedSubstr(std::string string, std::string flag, std::string key);

// This function will return the index of the next occurance of the given flag. "enable_escape_char" determines whether flags preceded
// by a backslash will be counted. This function behaves like std::string::find but allows for suppression behavior.
extern size_t p_getNext(std::string string, char flag, p_param parameters = P_DEFAULT);
extern size_t p_getNext(std::string string, size_t pos, char flag, p_param parameters = P_DEFAULT);
extern size_t p_getNext(std::string string, std::string flag, p_param parameters = P_DEFAULT);
extern size_t p_getNext(std::string string, size_t pos, std::string flag, p_param parameters = P_DEFAULT);
// This function takes a list of flags and returns the flag that occurs first. If no flags occur, it returns a null character.
extern char p_getFirstOf(std::string string, std::initializer_list<char> flags, p_param parameters = P_DEFAULT);
extern char p_getFirstOf(std::string string, size_t pos, std::initializer_list<char> flags, p_param parameters = P_DEFAULT);
// This function takes a list of flags and returns the flag that occurs first. If no flags occur, it returns an empty string.
extern std::string p_getFirstOf(std::string string, std::initializer_list<std::string> flags, p_param parameters = P_DEFAULT);
extern std::string p_getFirstOf(std::string string, size_t pos, std::initializer_list<std::string> flags, 
                                p_param parameters = P_DEFAULT);
// This function takes a list of flags and returns the index of the flag that occurs first.
extern size_t p_getFirstIndexOf(std::string string, std::initializer_list<char> flags, p_param parameters = P_DEFAULT);
extern size_t p_getFirstIndexOf(std::string string, size_t pos, std::initializer_list<char> flags, p_param parameters = P_DEFAULT);
extern size_t p_getFirstIndexOf(std::string string, std::initializer_list<std::string> flags, p_param parameters = P_DEFAULT);
extern size_t p_getFirstIndexOf(std::string string, size_t pos, std::initializer_list<std::string> flags, 
                                p_param parameters = P_DEFAULT);

// This function returns the index of the last occurrance of a string flag. NOTE: There is a function std::string::find_last_of but it
// doesn't work for strings.
extern size_t p_getLast(std::string string, std::string flag, p_param parameters = P_DEFAULT);

// This function takes a break flag and returns the substr until from the beginning of the string until the flag
extern std::string p_getNextSubstr(std::string string, char breaker, p_param parameters = P_DEFAULT);
extern std::string p_getNextSubstr(std::string string, size_t pos, char breaker, p_param parameters = P_DEFAULT);
extern std::string p_getNextSubstr(std::string string, std::string breaker, p_param parameters = P_DEFAULT);
extern std::string p_getNextSubstr(std::string string, size_t pos, std::string breaker, p_param parameters = P_DEFAULT);

// This function returns a substr of text that lies between two of the specified flags. "Inclusive" determines whether the flags
// themselves will be returned.
extern std::string p_getFlaggedSubstr(std::string string, char flag, p_param parameters = P_DEFAULT);
extern std::string p_getFlaggedSubstr(std::string string, size_t pos, char flag, p_param parameters = P_DEFAULT);
extern std::string p_getFlaggedSubstr(std::string string, std::string flag, p_param parameters = P_DEFAULT);
extern std::string p_getFlaggedSubstr(std::string string, size_t pos, std::string flag, p_param parameters = P_DEFAULT);
// This function takes a list of open and closing flag pairs and returns the substring of text that lies between the occurance of the 
// first opening flag and the flag that closes it, accounting for all potential nested flags in between. "Inclusive" determines whether
// the flags themselves will be returned.
extern std::string p_getFlaggedSubstr(std::string string, std::initializer_list<std::tuple<char, char>> flags, 
                                      p_param parameters = P_DEFAULT);
extern std::string p_getFlaggedSubstr(std::string string, size_t pos, std::initializer_list<std::tuple<char, char>> flags, 
                                      p_param parameters = P_DEFAULT);
// This function takes a list of open and closing flag pairs and returns the substring of text that lies between the occurance of the 
// first opening flag and the flag that closes it, accounting for all potential nested flags in between. "Inclusive" determines whether
// the flags themselves will be returned.
// NOTE: Take care to define flags carefully as unclear flags can result in extraneous flags. E.g., consider the pair "@@" and "@#". 
// If these flags appear together like so, "@@@#", then the parser will find 3 flags: "@@", "@@", and "@#".
extern std::string p_getFlaggedSubstr(std::string string, std::initializer_list<std::tuple<std::string, std::string>> flags, 
                                      p_param parameters = P_DEFAULT);
extern std::string p_getFlaggedSubstr(std::string string, size_t pos, std::initializer_list<std::tuple<std::string, std::string>> flags, 
                                      p_param parameters = P_DEFAULT);

extern std::string p_removeFlagged(std::string string, std::string o_flag, std::string c_flag, 
                                   p_param parameters = P_DEFAULT);
// Remove all of the given symbols from a string.
extern std::string p_removeAll(std::string string, std::initializer_list<char> symbols);
extern std::string p_removeAll(std::string string, size_t pos, std::initializer_list<char> symbols);
// Remove all of the given symbols from a string, except parts of the string that are flagged by the given flag. "Inclusive" determines
// whether the flags themselves will be returned.
extern std::string p_removeAllExceptFlagged(std::string string, std::initializer_list<char> symbols, char flag, 
                                            p_param parameters = P_DEFAULT);
extern std::string p_removeAllExceptFlagged(std::string string, size_t pos, std::initializer_list<char> symbols, char flag, 
                                            p_param parameters = P_DEFAULT);

// This function takes a string index and returns the substring surrounding that index (mainly intended for debugging purposes).
extern std::string p_getSnippet(std::string string, size_t pos);

#endif