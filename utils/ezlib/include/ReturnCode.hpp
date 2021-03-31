#ifndef __EZ_RETURN_CODE_HPP__
#define __EZ_RETURN_CODE_HPP__

#include <tuple>

namespace ez {

class ReturnCode
{
public:
    ReturnCode(int code = 0, int status = 0): _returnCode(code, status) { }

    // make it easier to use in if condition
    explicit operator bool() const { return IsOK(); }

    bool IsOK() const { return GetCode() == 0; }

    bool IsFail() const { return GetCode() != 0; }

    int GetCode() const { return std::get<0>(_returnCode); }

    int GetStatus() const { return std::get<1>(_returnCode); }

private:
    // 1st member as error step in function, 0 is ok
    // 2nd member as error code in libuv
    std::tuple<int, int> _returnCode;
};

} // namespace ez

#endif // __EZ_RETURN_CODE_HPP__
