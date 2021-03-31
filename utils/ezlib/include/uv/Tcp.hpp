#ifndef __EZ_UV_TCP_HPP__
#define __EZ_UV_TCP_HPP__

#include <vector>
#include "uv/Loop.hpp"

namespace ez { namespace uv {

class Tcp;
struct TcpCallBacks {
    /// (UVTcp's this, err type, msg), when error happen
    std::function<void(Tcp*, int, const char*)> OnError;

    /// (UVTcp's this), when connected(client connect) or new connection(server listen)
    std::function<void(Tcp*)> OnConnection;

    /// (UVTcp's this), when uv_close's callback is called
    std::function<void(Tcp*)> OnClosed;

    /// (UVTcp's this, msg, size), when new message
    std::function<void(Tcp*, const char*, std::size_t)> OnRead;

    /// (UVTcp's this, context), when write done
    std::function<void(Tcp*, void *)> OnWrite;

    /// test if all callback are set
    bool IsAllSet() const
    {
        return OnError && OnConnection && OnClosed && OnRead && OnWrite;
    }
};
    
class Tcp
{
public:
    // get string with format "ip:port" from sockaddr_in, you can control with only ip or port, default is both
    static std::string SockAddrToString(sockaddr_in addr, bool withIP = true, bool withPort = true);
    // get ip from domain name
    static std::vector<std::string> GetIp(uv_loop_t *loop, const std::string &node);
    // get port from service name
    static int GetPort(uv_loop_t *loop, const std::string &service);
    
    enum ErrType {
        ET_CONNECT,
        ET_CONNECT_CANCEL,
        ET_CONNECTION,
        ET_EOF,
        ET_ON_READ,
        ET_ON_WRITE,
        // when write request is canceled, cancel error can have a lots if you write a lot msgs and immediately close connectioin
        ET_ON_WRTTE_CANCEL
    };
    
    enum {
        READ_CACHE_SIZE = 64 * 1024
    };
    
    Tcp();
    ~Tcp();
    
    ReturnCode Init(uv_loop_t *loop, TcpCallBacks cb);
    
    /// get native socket handle, you can get it after connected
    uv_os_sock_t GetNativeSocket();
    /// get callbacks
    TcpCallBacks &CallBacks() { return _callbacks; }
    /// used as read buffer, should not be used outside
    char *_GetReadCache() { return _readCache; }
    
    ReturnCode NoDelay(bool noDelay);
    ReturnCode KeepAlive(bool keepAlive, std::size_t seconds);
    ReturnCode SetSendBufferSize(std::size_t size);
    ReturnCode SetReadBufferSize(std::size_t size);
    
    /// get local address in format "ip:port"
    std::string GetSockName() const;
    /// get remote address in format "ip:port"
    std::string GetPeerName() const;
    
    /// connect to remote addr, used as client
    ReturnCode Connect(const std::string& ip, int port);
    
    /// bind to addr, used as server
    ReturnCode Bind(const std::string& ip, int port);

    /// start listen for new connection, used as server
    ReturnCode Listen();

    /// accept new UVTcp
    ReturnCode Accept(Tcp& tcp);
    
    /// close tcp handle
    void Close();
    
    /// start reading from socket
    ReturnCode StartRead();
    
    /// async write to tcp stream, you should release buffer after OnWrite callback
    ReturnCode Write(uv_buf_t* bufArray, std::size_t size, void *context);
    
private:

    uv_tcp_t *_uvtcp;
    TcpCallBacks _callbacks;
    char _readCache[READ_CACHE_SIZE];
};

} } // namespace ez::uv

#endif // __EZ_UV_TCP_HPP__
