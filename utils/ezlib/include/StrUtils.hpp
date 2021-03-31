#ifndef __EZ_STR_UTILS_HPP__
#define __EZ_STR_UTILS_HPP__

#include <string>
#include <vector>
#include <map>
#include <initializer_list>

namespace ez {

// when delimiterCompress is true, adjacent separators are merged together. Otherwise, every two separators delimit a token
std::vector<std::string> Tokenize(const std::string &input,
                                  std::initializer_list<char> delimiters = {' '},
                                  bool delimiterCompress = true);

void TrimLeft(std::string &target, std::initializer_list<char> trimTargets = { ' ' });
void TrimRight(std::string &target, std::initializer_list<char> trimTargets = { ' ' });
void Trim(std::string &target, std::initializer_list<char> trimTargets = { ' ' });

// used to parse key/value like "id=1234,price=12.25", store key as string
std::map<std::string, std::string> ParseKeyValue(const std::string &input, char seperator = '\001', char assignment = '=');
std::map<std::string, std::string> ParseKeyValue(const char *data, std::size_t size, char seperator = '\001', char assignment = '=');

// used to parse key/value like "35=0,111=abc", store key as int
std::map<int, std::string> ParseIKeyValue(const std::string &input, char seperator = '\001', char assignment = '=');
std::map<int, std::string> ParseIKeyValue(const char *data, std::size_t size, char seperator = '\001', char assignment = '=');

} // namespace ez

#endif // __EZ_STR_UTILS_HPP__
