#include "uv/Tcp.hpp"
#include <cstring>

namespace ez { namespace uv {

std::string SockAddrToString(sockaddr_in addr, bool withIP, bool withPort)
{
    std::string result;
    if (withIP)
        result += (inet_ntoa(addr.sin_addr));
    
    if (withIP && withPort)
        result += ':';
    
    if (withPort)
    {
        int port = ntohs(addr.sin_port);
        result += std::to_string(port);
    }
    return result;
}
    
std::vector<std::string> GetIp(uv_loop_t *loop, const std::string &node)
{
    uv_getaddrinfo_t req;
    
    addrinfo hints;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    std::vector<std::string> results;
    int status = uv_getaddrinfo(loop, &req, NULL, node.c_str(), NULL, &hints);
    if (status == 0)
    {
        addrinfo *addrList = req.addrinfo;
        do
        {
            sockaddr_in *addr = (sockaddr_in *)addrList->ai_addr;
            std::string ip = SockAddrToString(*addr, true, false);
            results.push_back(ip);
            addrList = addrList->ai_next;
        } while (addrList != NULL);
        
        uv_freeaddrinfo(req.addrinfo);
    }
    
    return results;
}

int GetPort(uv_loop_t *loop, const std::string &service)
{
    uv_getaddrinfo_t req;
    
    addrinfo hints;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    int status = uv_getaddrinfo(loop, &req, NULL, NULL, service.c_str(), &hints);
    if (status == 0)
    {
        addrinfo *addrList = req.addrinfo;
        sockaddr_in *addr = (sockaddr_in *)addrList->ai_addr;
        int port = ntohs(addr->sin_port);
        
        uv_freeaddrinfo(req.addrinfo);
        return port;
    }
    else
        return 0;
}

Tcp::Tcp(): _uvtcp(nullptr)
{

}

Tcp::~Tcp()
{
    
}

ReturnCode Tcp::Init(uv_loop_t *loop, TcpCallBacks cb)
{
    if (!cb.IsAllSet())
        return {-1, 0};
    
    _uvtcp = new uv_tcp_t;
    int status = uv_tcp_init(loop, _uvtcp);
    if (status == 0)
    {
        uv_handle_set_data((uv_handle_t*)_uvtcp, this);
        _callbacks = cb;
        return {};
    }
    else
    {
        delete _uvtcp;
        _uvtcp = nullptr;
        return {-2, status};
    }
}

uv_os_sock_t Tcp::GetNativeSocket()
{
    uv_os_fd_t fd(-1);
    int status = uv_fileno((uv_handle_t *)_uvtcp, &fd);
    return (status == 0) ? fd : -1;
}

ReturnCode Tcp::NoDelay(bool noDelay)
{
    int status = uv_tcp_nodelay(_uvtcp, noDelay ? 1 : 0);
    return (status == 0) ? ReturnCode() : ReturnCode(-1, status);
}

ReturnCode Tcp::KeepAlive(bool keepAlive, std::size_t seconds)
{
    int status = uv_tcp_keepalive(_uvtcp, keepAlive ? 1 : 0, seconds);    
    return (status == 0) ? ReturnCode() : ReturnCode(-1, status);
}

ReturnCode Tcp::SetSendBufferSize(std::size_t size)
{
    int value = (int)size;
    int status = uv_send_buffer_size((uv_handle_t *)_uvtcp, &value);
    return (status == 0) ? ReturnCode() : ReturnCode(-1, status);
}

ReturnCode Tcp::SetReadBufferSize(std::size_t size)
{
    int value = (int)size;
    int status = uv_recv_buffer_size((uv_handle_t *)_uvtcp, &value);
    return (status == 0) ? ReturnCode() : ReturnCode(-1, status);
}

std::string Tcp::GetSockName() const
{
    sockaddr_in addr;
    int len = sizeof(sockaddr_in);
    memset(&addr, 0, len);

    int status = uv_tcp_getsockname(_uvtcp, (sockaddr*)&addr, &len);
    if (status != 0) {
        return std::string();
    }

    return SockAddrToString(addr);
}

std::string Tcp::GetPeerName() const
{
    sockaddr_in addr;
    int len = sizeof(sockaddr_in);
    memset(&addr, 0, len);

    int status = uv_tcp_getpeername(_uvtcp, (sockaddr*)&addr, &len);
    if (status != 0) {
        return std::string();
    }

    return SockAddrToString(addr);    
}

ReturnCode Tcp::Connect(const std::string& ip, int port)
{
    sockaddr_in dest;
    int status = uv_ip4_addr(ip.c_str(), port, &dest);
    if (status != 0)
        return ReturnCode(-1, status);

    uv_connect_t* connectReq = new uv_connect_t;
    uv_req_set_data((uv_req_t *)connectReq, this);
    
    status = uv_tcp_connect(connectReq, _uvtcp, (const sockaddr*)&dest,
                [] (uv_connect_t* req, int status)
                {
                    Tcp *tcp = (Tcp *)uv_req_get_data((uv_req_t *)req);
                    delete req;

                    if (status != 0) 
                    {
                        if (status == UV_ECANCELED)
                            tcp->CallBacks().OnError(tcp, Tcp::ET_CONNECT_CANCEL, uv_strerror(status));
                        else
                            tcp->CallBacks().OnError(tcp, Tcp::ET_CONNECT, uv_strerror(status));
                    }
                    else 
                    {
                        tcp->CallBacks().OnConnection(tcp);
                        tcp->StartRead();
                    }
                }
            );

    if (status == 0)
        return {};
    else
    {
        delete connectReq;
        return ReturnCode(-2, status);
    }
}

ReturnCode Tcp::Bind(const std::string& ip, int port)
{
    sockaddr_in addr;
    int status = uv_ip4_addr(ip.c_str(), port, &addr);
    if (status != 0)
        return ReturnCode(-1, status);

    status = uv_tcp_bind(_uvtcp, (const struct sockaddr*)&addr, 0);
    if (status != 0)
        return {-1, status};
    else
        return {};
}

ReturnCode Tcp::Listen()
{
    int status = uv_listen((uv_stream_t*)_uvtcp,
                           SOMAXCONN,
                           [] (uv_stream_t *server, int status)
                           {
                                Tcp *tcp = (Tcp *)uv_handle_get_data((uv_handle_t*)server);
                                if (status != 0) 
                                    tcp->CallBacks().OnError(tcp, Tcp::ET_CONNECTION, uv_strerror(status));
                                else 
                                    tcp->CallBacks().OnConnection(tcp);
                           });

    if (status != 0)
        return {-1, status};
    else
        return {};
}

ReturnCode Tcp::Accept(Tcp& tcp)
{
    int status = uv_accept((uv_stream_t*)_uvtcp, (uv_stream_t*)(tcp._uvtcp));
    if (status != 0)
        return {-1, status};
    else
        return {};
}

void Tcp::Close()
{
    if (_uvtcp == nullptr)
        return;

    uv_close((uv_handle_t*)_uvtcp,
            [] (uv_handle_t* handle)
            {
                Tcp *tcp = (Tcp *)uv_handle_get_data(handle);
                tcp->CallBacks().OnClosed(tcp);
                delete handle;
            });
}

ReturnCode Tcp::StartRead()
{
    int status = uv_read_start((uv_stream_t*)_uvtcp,
            [] (uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
            {
                Tcp *tcp = (Tcp *)uv_handle_get_data(handle);
                buf->base = tcp->_GetReadCache();
                buf->len = Tcp::READ_CACHE_SIZE;
            },
            [] (uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
            {
                Tcp *tcp = (Tcp *)uv_handle_get_data((uv_handle_t *)stream);
                if (nread > 0) 
                {
                    tcp->CallBacks().OnRead(tcp, buf->base, nread);
                }
                else if (nread < 0) 
                {
                    uv_read_stop(stream);

                    if (nread == UV_EOF)
                        tcp->CallBacks().OnError(tcp, Tcp::ET_EOF, uv_strerror(nread));
                    else
                        tcp->CallBacks().OnError(tcp, Tcp::ET_ON_READ, uv_strerror(nread));
                }
            });

    if (status != 0)
        return {-1, status};
    else
        return {};
}

ReturnCode Tcp::Write(uv_buf_t* bufArray, std::size_t size, void *context)
{
    uv_write_t* writeReq = new uv_write_t;
    uv_req_set_data((uv_req_t *)writeReq, context);

    int status = uv_write(writeReq, (uv_stream_t*)_uvtcp, bufArray, size,
                        [] (uv_write_t* req, int status)
                        {
                            Tcp *tcp = (Tcp *)uv_handle_get_data((uv_handle_t *)req->handle);
                            void *context = uv_req_get_data((uv_req_t *)req);
                            delete req;
                            if (status != 0)
                            {
                                if (status == UV_ECANCELED)
                                    tcp->CallBacks().OnError(tcp, Tcp::ET_ON_WRTTE_CANCEL, uv_strerror(status));
                                else
                                    tcp->CallBacks().OnError(tcp, Tcp::ET_ON_WRITE, uv_strerror(status));
                            }

                            // notify user event write fail, for they to release write buffer
                            tcp->CallBacks().OnWrite(tcp, context);
                        });

    if (status != 0)
    {
        delete writeReq;
        return {-1, status};
    }
    else
        return {};
}

} } // namespace ez::uv
