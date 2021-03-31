#include "StrUtils.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/classification.hpp"
#include "boost/algorithm/string/trim.hpp"
#include "boost/spirit/include/qi.hpp"
#include "boost/fusion/adapted/std_pair.hpp"

namespace ez {

std::vector<std::string> Tokenize(const std::string &input, std::initializer_list<char> delimiters, bool delimiterCompress)
{
    std::vector<std::string> tokens;

    boost::algorithm::split(tokens, input,
            boost::is_any_of(delimiters),
//            [delimiter] (const char &ch) { return ch == delimiter; },
            delimiterCompress ? boost::algorithm::token_compress_on : boost::algorithm::token_compress_off);
    
    return tokens;
}

void TrimLeft(std::string &target, std::initializer_list<char> trimTargets)
{
    boost::algorithm::trim_left_if(target, boost::is_any_of(trimTargets));
}

void TrimRight(std::string &target, std::initializer_list<char> trimTargets)
{
    boost::algorithm::trim_right_if(target, boost::is_any_of(trimTargets));
}

void Trim(std::string &target, std::initializer_list<char> trimTargets)
{
    boost::algorithm::trim_if(target, boost::is_any_of(trimTargets));
}

typedef std::map<std::string, std::string> KVMap;

template <typename Iterator>
struct KeyValueParser : boost::spirit::qi::grammar<Iterator, KVMap()>
{
    KeyValueParser(char seperator, char assignment): KeyValueParser::base_type(query)
    {
        key = +(boost::spirit::qi::char_ - assignment - seperator);

        value = +(boost::spirit::qi::char_ - assignment - seperator);

        keyvalue = key >> assignment >> value;

        query = keyvalue >> *(boost::spirit::qi::lit(seperator) >> keyvalue) >> -boost::spirit::qi::lit(seperator);
    }

    boost::spirit::qi::rule<Iterator, std::string()> key;
    boost::spirit::qi::rule<Iterator, std::string()> value;
    boost::spirit::qi::rule<Iterator, std::pair<std::string, std::string>> keyvalue;
    boost::spirit::qi::rule<Iterator, KVMap()> query;
};

std::map<std::string, std::string> ParseKeyValue(const std::string &input, char seperator, char assignment)
{
    std::map<std::string, std::string> result = ParseKeyValue(input.c_str(), input.size(), seperator, assignment);
    return result;
}

std::map<std::string, std::string> ParseKeyValue(const char *data, std::size_t size, char seperator, char assignment)
{
    std::map<std::string, std::string> result;
    
    KeyValueParser<const char *> parser(seperator, assignment);
    
    boost::spirit::qi::parse(data, data + size, parser, result);
    
    return result;
}

typedef std::map<int, std::string> IKVMap;

template <typename Iterator>
struct IKeyValueParser : boost::spirit::qi::grammar<Iterator, IKVMap()>
{
    IKeyValueParser(char seperator, char assignment): IKeyValueParser::base_type(query)
    {
        key = boost::spirit::qi::int_;

        value = +(boost::spirit::qi::char_ - assignment - seperator);

        keyvalue = key >> assignment >> value;

        query = keyvalue >> *(boost::spirit::qi::lit(seperator) >> keyvalue) >> -boost::spirit::qi::lit(seperator);
    }

    boost::spirit::qi::rule<Iterator, int> key;
    boost::spirit::qi::rule<Iterator, std::string()> value;
    boost::spirit::qi::rule<Iterator, std::pair<int, std::string>> keyvalue;
    boost::spirit::qi::rule<Iterator, IKVMap()> query;
};

std::map<int, std::string> ParseIKeyValue(const std::string &input, char seperator, char assignment)
{
    std::map<int, std::string> result = ParseIKeyValue(input.c_str(), input.size(), seperator, assignment);
    return result;
}

std::map<int, std::string> ParseIKeyValue(const char *data, std::size_t size, char seperator, char assignment)
{
    std::map<int, std::string> result;
    
    IKeyValueParser<const char *> parser(seperator, assignment);
    
    boost::spirit::qi::parse(data, data + size, parser, result);
    
    return result; 
}

} // namespace ez