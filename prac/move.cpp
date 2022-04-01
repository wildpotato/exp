#include <iostream>
#include <cstring>
#include <utility>

using std::cout;
using std::endl;

class rule_of_five
{
    char* cstring; // raw pointer used as a handle to a dynamically-allocated memory block
public:
    rule_of_five(const char* s = "") : cstring(nullptr)
    {
        if (s)
        {
            std::size_t n = std::strlen(s) + 1;
			cout << "Create\n";
            cstring = new char[n];      // allocate
            std::memcpy(cstring, s, n); // populate
        }
    }

    ~rule_of_five()
    {
		cout << "Destroy\n";
        delete[] cstring; // deallocate
    }

    rule_of_five(const rule_of_five& other) // copy constructor
    : rule_of_five(other.cstring)
	{
		cout << "Copy\n";
	}

    rule_of_five(rule_of_five&& other) noexcept // move constructor
    : cstring(std::exchange(other.cstring, nullptr))
	{
		cout << "Move\n";
	}

    rule_of_five& operator=(const rule_of_five& other) // copy assignment
    {
		cout << "Copy assign\n";
        return *this = rule_of_five(other);
    }

    rule_of_five& operator=(rule_of_five&& other) noexcept // move assignment
    {
		cout << "Move assign\n";
        std::swap(cstring, other.cstring);
        return *this;
    }

	void display()
	{
		cout << cstring << endl;
	}
// alternatively, replace both assignment operators with
//  rule_of_five& operator=(rule_of_five other) noexcept
//  {
//      std::swap(cstring, other.cstring);
//      return *this;
//  }
};

int main() {
	rule_of_five r1("Hello");
	rule_of_five r2 = r1;
	return 0;
}
