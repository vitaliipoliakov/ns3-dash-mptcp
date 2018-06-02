#ifndef HTTP_SERVER_FAKE_CLIENTSOCKET
#define HTTP_SERVER_FAKE_CLIENTSOCKET


#include "ns3/callback.h"
#include "ns3/application.h"
#include "ns3/ptr.h"
#include "ns3/string.h"
#include "ns3/tcp-socket.h"

#include <map>
#include <vector>
#include <stdio.h>


namespace ns3
{
class Socket;
class Address;


class HttpServerFakeClientSocket
{
public:
  HttpServerFakeClientSocket(uint64_t socket_id,
  std::string contentDir, std::map<std::string /* filename */, long /* file size */>& fileSizes,
  std::vector<std::string /* filename */>& virtualFiles,
  Callback<void, uint64_t> finished_callback);

  virtual ~HttpServerFakeClientSocket();


  void HandleIncomingData(Ptr<Socket> socket);

  void HandleReadyToTransmit(Ptr<Socket> socket, uint32_t txSize);


  void ConnectionClosedNormal (Ptr<Socket> socket);
  void ConnectionClosedError (Ptr<Socket> socket);


  void LogStateChange(const ns3::TcpSocket::TcpStates_t old_state, const ns3::TcpSocket::TcpStates_t new_state);
  void LogCwndChange(uint32_t oldCwnd, uint32_t newCwnd);


protected:
  Callback<void, uint64_t> m_finished_callback;

  virtual void FinishedIncomingData(Ptr<Socket> socket, Address from, std::string data);
  void AddBytesToTransmit(const uint8_t* buffer, uint32_t size);

  std::string ParseHTTPHeader(std::string data);

  long GetFileSize(std::string filename);



protected:
  std::string m_content_dir;

  uint64_t m_socket_id;
  uint32_t bytes_recv;
  uint32_t bytes_sent;

  uint32_t m_totalBytesToTx;
  uint32_t m_currentBytesTx;

  bool m_is_shutdown;

  bool m_is_virtual_file;

  bool m_keep_alive;

  std::vector<uint8_t> m_bytesToTransmit;


  std::string m_activeRecvString;

  std::map<std::string,long>& m_fileSizes;
  std::vector<std::string>& m_virtualFiles;
};

} // namespace ns3


#endif /* HTTP_SERVER_FAKE_CLIENTSOCKET */
