#ifndef HTTP_SERVER_FAKE_VIRTUAL_CLIENTSOCKET
#define HTTP_SERVER_FAKE_VIRTUAL_CLIENTSOCKET

#include "http-server-fake-clientsocket.h"

namespace ns3
{
class Socket;
class Address;


class HttpServerFakeVirtualClientSocket : public HttpServerFakeClientSocket
{
public:
  HttpServerFakeVirtualClientSocket(uint64_t socket_id,
  std::string contentDir, std::map<std::string /* filename */, long /* file size */>& fileSizes,
  std::vector<std::string /* filename */>& fakeFiles, std::map<std::string,std::string>& /* virtual file host */ virtualHostedFiles,
  Callback<void, uint64_t> finished_callback);

  ~HttpServerFakeVirtualClientSocket();

protected:
  std::map<std::string,std::string>& m_virtualHostedFiles;

  void FinishedIncomingData(Ptr<Socket> socket, Address from, std::string data);


};

} // namespace ns3


#endif /* HTTP_SERVER_FAKE_VIRTUAL_CLIENTSOCKET */
