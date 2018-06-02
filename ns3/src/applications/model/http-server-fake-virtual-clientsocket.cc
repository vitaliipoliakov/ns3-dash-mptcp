#include "http-server-fake-virtual-clientsocket.h"

#include "http-server-fake-clientsocket.h"

#include <fstream>
#include <algorithm>

#include "ns3/socket.h"
#include "ns3/packet.h"


#include <sys/types.h>
#include <sys/stat.h>

#define CRLF "\r\n"

namespace ns3
{



HttpServerFakeVirtualClientSocket::HttpServerFakeVirtualClientSocket(uint64_t socket_id,
    std::string contentDir,
    std::map<std::string /* filename */, long /* file size */>& fileSizes,
    std::vector<std::string /* filename */>& fakeFiles,
    std::map<std::string,std::string>& /* virtual file host */ virtualHostedFiles,
    Callback<void, uint64_t> finished_callback) :
     HttpServerFakeClientSocket(socket_id, contentDir, fileSizes, fakeFiles, finished_callback),
     m_virtualHostedFiles(virtualHostedFiles)
{

}



HttpServerFakeVirtualClientSocket::~HttpServerFakeVirtualClientSocket()
{
  fprintf(stderr, "Server(%ld): Destructing Fake Virtual Client Socket(%ld)...\n", m_socket_id, m_socket_id);
  this->m_bytesToTransmit.clear();
}




void
HttpServerFakeVirtualClientSocket::FinishedIncomingData(Ptr<Socket> socket, Address from, std::string data)
{
  fprintf(stderr, "VirtualServer(%ld)::FinishedIncomingData(socket,data=str(%ld))\n", m_socket_id, data.length());
  // now parse this request (TODO) and reply
  std::string filename = m_content_dir  + ParseHTTPHeader(data);

  fprintf(stderr, "VirtualServer(%ld): Request Opening '%s'\n", m_socket_id, filename.c_str());

  long filesize = GetFileSize(filename);

  if (filesize == -1)
  {
    fprintf(stderr, "VirtualServer(%ld): Error, '%s' not found!\n", m_socket_id, filename.c_str());
    // return 404
    std::string replyString("HTTP/1.1 404 Not Found\r\n\r\n");

    AddBytesToTransmit((uint8_t*)replyString.c_str(), replyString.length());
  } else
  {
    // Create a proper header
    std::stringstream replySS;
    replySS << "HTTP/1.1 200 OK" << CRLF; // OR HTTP/1.1 404 Not Found
    replySS << "Content-Type: text/xml; charset=utf-8" << CRLF; // e.g., when sending the MPD
    replySS << "Content-Length: " << filesize << CRLF;
    replySS << CRLF;

    std::string replyString = replySS.str();
    uint8_t* buffer = (uint8_t*)replyString.c_str();
    AddBytesToTransmit(buffer,replyString.length());

    // now append the virtual payload data
    uint8_t tmp[4096];



    if (std::find(m_virtualFiles.begin(), m_virtualFiles.end(), filename) != m_virtualFiles.end())
    {
      // handle virtual payload
      // fill tmp with some random data
      fprintf(stderr, "VirtualServer(%ld): Generating virtual payload with size %ld ...\n", m_socket_id, filesize);
      for (int i = 0; i < 4096; i++)
      {
        tmp[i] = (uint8_t)rand();
      }


      int cnt = 0;

      while (cnt < filesize)
      {
        if (cnt + 4096 < filesize)
        {
          AddBytesToTransmit(tmp, 4096);
        } else {
          AddBytesToTransmit(tmp, filesize - cnt);
        }
        cnt += 4096;
      }
    } else if (m_virtualHostedFiles.find(filename) != m_virtualHostedFiles.end())
    {
      fprintf(stderr, "VirtualServer(%ld): Opening file in memory with size %ld ...\n", m_socket_id, filesize);
      // handle actual payload

      std::string bytes_memory = m_virtualHostedFiles[filename];

      AddBytesToTransmit((const uint8_t*)bytes_memory.c_str(),bytes_memory.length());

    } else
    {
      fprintf(stderr, "VirtualServer(%ld): Opening file on disk with size %ld ...\n", m_socket_id, filesize);
      // handle actual payload
      FILE* fp = fopen(filename.c_str(), "rb");

      int size_returned = 4096;

      while (size_returned == 4096)
      {
        size_returned = fread(tmp, 1, 4096, fp);

        if (size_returned > 0)
        {
          AddBytesToTransmit(tmp, size_returned);
        }
      }

      fclose(fp);
    }
  }


  HandleReadyToTransmit(socket, socket->GetTxAvailable());
}



};
