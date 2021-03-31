#include "catch.hpp"
#include <vector>
#include <list>
#include <map>
#include "StackAllocator.hpp"

// Create a vector<T> template with a small buffer of 200 bytes.
//   Note for vector it is possible to reduce the alignment requirements
//   down to alignof(T) because vector doesn't allocate anything but T's.
//   And if we're wrong about that guess, it is a compile-time error, not
//   a run time error.
template <class T, std::size_t BufSize = 256>
using SmallVector = std::vector<T, ez::StackAllocator<T, BufSize, alignof(T)>>;

TEST_CASE("use StackAllocator on vector", "[ez][StackAllocator]")
{    
    SmallVector<int>::allocator_type::area_type a;
    SmallVector<int> v{a};

    v.push_back(1);
    REQUIRE(v.size() == 1);
    REQUIRE(v[0] == 1);

    v.push_back(2);
    REQUIRE(v.size() == 2);
    REQUIRE(v[1] == 2);
    
    v.push_back(3);
    REQUIRE(v.size() == 3);
    REQUIRE(v[2] == 3);
    
    v.push_back(4);
    REQUIRE(v.size() == 4);
    REQUIRE(v[3] == 4);
}

template <class T, std::size_t BufSize = 256>
using SmallList = std::list<T, ez::StackAllocator<T, BufSize>>;

TEST_CASE("use StackAllocator on list", "[ez][StackAllocator]")
{    
    SmallList<int>::allocator_type::area_type a;
    SmallList<int> list{a};

    list.push_back(1);
    REQUIRE(list.size() == 1);
    REQUIRE(list.back() == 1);

    list.push_back(2);
    REQUIRE(list.size() == 2);
    REQUIRE(list.back() == 2);
    
    list.push_back(3);
    REQUIRE(list.size() == 3);
    REQUIRE(list.back() == 3);
    
    list.push_back(4);
    REQUIRE(list.size() == 4);
    REQUIRE(list.back() == 4);
}

template <class Key, class T, std::size_t BufSize = 256>
using SmallMap = std::map<Key, T, std::less<Key>, ez::StackAllocator<std::pair<const Key, T>, BufSize>>;
TEST_CASE("use StackAllocator on map", "[ez][StackAllocator]")
{    
    SmallMap<int, int>::allocator_type::area_type a;
    SmallMap<int, int> map{a};

    map[1] = 5;
    map[2] = 6;
    map[3] = 7;
    map[4] = 8;
    
    REQUIRE(map.size() == 4);
    REQUIRE(map[1] == 5);
    REQUIRE(map[2] == 6);
    REQUIRE(map[3] == 7);
    REQUIRE(map[4] == 8);
}