#include "catch.hpp"
#include "StrUtils.hpp"

using namespace ez;

TEST_CASE("tokenize - empty string", "[ez][StrUtils]")
{
    std::string input;
    
    auto tokens = Tokenize(input);

    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0] == "");
}

TEST_CASE("tokenize - delimiter at begin", "[ez][StrUtils]")
{
    std::string input(",abc");
    
    auto tokens = Tokenize(input, {','});

    REQUIRE(tokens.size() == 2);
    REQUIRE(tokens[0] == "");
    REQUIRE(tokens[1] == "abc");
}

TEST_CASE("tokenize - delimiter at end", "[ez][StrUtils]")
{
    std::string input("abc,");
    
    auto tokens = Tokenize(input, {','});

    REQUIRE(tokens.size() == 2);
    REQUIRE(tokens[0] == "abc");
    REQUIRE(tokens[1] == "");
}

TEST_CASE("tokenize - delimiter not exists", "[ez][StrUtils]")
{
    std::string input("abc def");
    
    auto tokens = Tokenize(input, {','});

    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0] == "abc def");
}

TEST_CASE("tokenize - default", "[ez][StrUtils]")
{
    std::string input("abc aa bbb ccc d");
    
    auto tokens = Tokenize(input);

    REQUIRE(tokens.size() == 5);
    REQUIRE(tokens[0] == "abc");
    REQUIRE(tokens[1] == "aa");
    REQUIRE(tokens[2] == "bbb");
    REQUIRE(tokens[3] == "ccc");
    REQUIRE(tokens[4] == "d");
}

TEST_CASE("tokenize - multi-delimiter", "[ez][StrUtils]")
{
    std::string input("abc aa b\tb c\t\t cc d");
    
    auto tokens = Tokenize(input,  { ' ', '\t' });

    REQUIRE(tokens.size() == 7);
    REQUIRE(tokens[0] == "abc");
    REQUIRE(tokens[1] == "aa");
    REQUIRE(tokens[2] == "b");
    REQUIRE(tokens[3] == "b");
    REQUIRE(tokens[4] == "c");
    REQUIRE(tokens[5] == "cc");
    REQUIRE(tokens[6] == "d");
}


TEST_CASE("tokenize - with delimiter compress", "[ez][StrUtils]")
{
    std::string input("aaa  bbb  ccc");
    
    auto tokens = Tokenize(input, {' '});

    REQUIRE(tokens.size() == 3);
    REQUIRE(tokens[0] == "aaa");
    REQUIRE(tokens[1] == "bbb");
    REQUIRE(tokens[2] == "ccc");
}

TEST_CASE("tokenize - without delimiter compress", "[ez][StrUtils]")
{
    std::string input("aaa  bbb  ccc");
    
    auto tokens = Tokenize(input, {' '}, false);

    REQUIRE(tokens.size() == 5);
    REQUIRE(tokens[0] == "aaa");
    REQUIRE(tokens[1] == "");
    REQUIRE(tokens[2] == "bbb");
    REQUIRE(tokens[3] == "");
    REQUIRE(tokens[4] == "ccc");
}


TEST_CASE("trim - default", "[ez][StrUtils]")
{
    std::string input("   hello world! ");
    
    TrimLeft(input);
    REQUIRE(input == "hello world! ");

    TrimRight(input);
    REQUIRE(input == "hello world!");
    
    input = "  abc def     ";
    Trim(input);
    REQUIRE(input == "abc def");    
}

TEST_CASE("trim - multi-target", "[ez][StrUtils]")
{
    std::string input(" \t  hello world! \t\n");
    
    TrimLeft(input, { ' ', '\t' });
    REQUIRE(input == "hello world! \t\n");

    TrimRight(input, { ' ', '\t', '\n' });
    REQUIRE(input == "hello world!");
    
    input = " \t  abc def  \t   ";
    Trim(input, { ' ', '\t' });
    REQUIRE(input == "abc def");    
}

TEST_CASE("ParseKeyValue - normal case", "[ez][StrUtils]")
{
    std::string input("aaa=bbb,ccc=ddd,");
    
    auto kvmap = ez::ParseKeyValue(input, ',');
    
    REQUIRE(kvmap.size() == 2);
    REQUIRE(kvmap["aaa"] == "bbb");
    REQUIRE(kvmap["ccc"] == "ddd");
}

TEST_CASE("ParseKeyValue - missing last delimiter", "[ez][StrUtils]")
{
    // it's ok without last delimiter
    std::string input("aaa=bbb,ccc=ddd");
    
    auto kvmap = ez::ParseKeyValue(input, ',');
    
    REQUIRE(kvmap.size() == 2);
    REQUIRE(kvmap["aaa"] == "bbb");
    REQUIRE(kvmap["ccc"] == "ddd");
}

TEST_CASE("ParseKeyValue - missing value", "[ez][StrUtils]")
{
    // parser stopped at last valid key/value
    std::string input("aaa=bbb,ccc=,ddd=eee");
    
    auto kvmap = ez::ParseKeyValue(input, ',');
    
    REQUIRE(kvmap.size() == 1);
    REQUIRE(kvmap["aaa"] == "bbb");
}

TEST_CASE("ParseIKeyValue - normal case", "[ez][StrUtils]")
{
    std::string input("11=bbb,22=ddd,");
    
    auto kvmap = ez::ParseIKeyValue(input, ',');
    
    REQUIRE(kvmap.size() == 2);
    REQUIRE(kvmap[11] == "bbb");
    REQUIRE(kvmap[22] == "ddd");
}

TEST_CASE("ParseIKeyValue - missing last delimiter", "[ez][StrUtils]")
{
    // it's ok without last delimiter
    std::string input("11=bbb,22=ddd");
    
    auto kvmap = ez::ParseIKeyValue(input, ',');
    
    REQUIRE(kvmap.size() == 2);
    REQUIRE(kvmap[11] == "bbb");
    REQUIRE(kvmap[22] == "ddd");
}

TEST_CASE("ParseIKeyValue - missing value", "[ez][StrUtils]")
{
    // parser stopped at last valid key/value
    std::string input("11=bbb,22=,33=eee");
    
    auto kvmap = ez::ParseIKeyValue(input, ',');
    
    REQUIRE(kvmap.size() == 1);
    REQUIRE(kvmap[11] == "bbb");
}

TEST_CASE("ParseIKeyValue - key invalid", "[ez][StrUtils]")
{
    // parser stopped at last valid key/value
    std::string input("11=bbb,abc=def,");
    
    auto kvmap = ez::ParseIKeyValue(input, ',');
    
    REQUIRE(kvmap.size() == 1);
    REQUIRE(kvmap[11] == "bbb");
}