#include <iostream>
#include "tbl_itoa.h"

using namespace std;

int main()
{
	for (auto iter = m_itoa.begin(); iter != m_itoa.end(); ++iter)
	{
		std::cout << iter->first << " -> " << iter->second << std::endl;
	}
	return 0;
}
