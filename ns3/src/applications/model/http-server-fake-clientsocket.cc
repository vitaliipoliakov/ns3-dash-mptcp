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



HttpServerFakeClientSocket::HttpServerFakeClientSocket(uint64_t socket_id,
    std::string contentDir,
    std::map<std::string /* filename */, long /* file size */>& fileSizes,
    std::vector<std::string /* filename */>& virtualFiles,
    Callback<void, uint64_t> finished_callback) : m_fileSizes(fileSizes), m_virtualFiles(virtualFiles)
{
  this->m_socket_id = socket_id;
  this->m_finished_callback = finished_callback;
  bytes_recv = 0;
  bytes_sent = 0;
  m_currentBytesTx = 0;
  m_totalBytesToTx = 0;
  m_activeRecvString = "";
  m_is_shutdown = false;
  m_content_dir = contentDir;

  m_keep_alive = false;

  m_is_virtual_file = false;
}



HttpServerFakeClientSocket::~HttpServerFakeClientSocket()
{
  fprintf(stderr, "Server(%ld): Destructing Client Socket(%ld)...\n", m_socket_id, m_socket_id);
  this->m_bytesToTransmit.clear();
}






void
HttpServerFakeClientSocket::HandleIncomingData(Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
  {
    packet->RemoveAllPacketTags ();
    packet->RemoveAllByteTags ();

    uint8_t* buffer = (uint8_t*) malloc ((size_t) packet->GetSize()+1);
    // PARSE PACKET
    size_t packet_size = packet->CopyData(buffer, packet->GetSize());
    buffer[packet_size] = '\0';

    // check if packet in m_activePackets
    if (bytes_recv == 0)
    {
      m_activeRecvString = std::string((char*)buffer);
      bytes_recv += packet_size;
    } else
    {
      m_activeRecvString += std::string((char*)buffer);
      bytes_recv += packet_size;
    }

    free(buffer);


    if (string_ends_width(m_activeRecvString, std::string(CRLF)))
    {
      m_currentBytesTx = 0;
      m_totalBytesToTx = 0;
      FinishedIncomingData(socket, from, m_activeRecvString);
    }
  }
}


std::string
HttpServerFakeClientSocket::ParseHTTPHeader(std::string data)
{
  const char* cBuffer = data.c_str();

  char needle1[5];
  strcpy(needle1, "GET ");
  needle1[4] = '\0';

  char needle2[2];
  needle2[0] = ' ';
  needle2[1] = '\0';

  // find the name that was requested
  const char* p1 = strstr(cBuffer, needle1);
  int pos1 = p1-cBuffer+4;
  const char* p2 = strstr(& (cBuffer[pos1]), needle2);


  int pos2 = p2 - &cBuffer[pos1];

  char actualFileName[256];

  strncpy(actualFileName, &cBuffer[pos1], pos2);
  actualFileName[pos2] = '\0';

  std::string sFilename(actualFileName);


  // check if keep-alive is set
  if (data.find("Connection: keep-alive") != std::string::npos)
  {
    this->m_keep_alive = true;
  }

  return sFilename;
}


void HttpServerFakeClientSocket::ConnectionClosedNormal(Ptr<Socket> socket)
{
  fprintf(stderr, "Server(%ld): Connection closing normally...\n", m_socket_id);
  // just in case, make sure the callbacks are no longer active

  // remove the send callback
  socket->SetSendCallback (MakeNullCallback<void, Ptr<Socket>, uint32_t > ());
  // remove the recv callback
  socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());

  this->m_finished_callback(this->m_socket_id);
}


void HttpServerFakeClientSocket::ConnectionClosedError(Ptr<Socket> socket)
{
  fprintf(stderr, "Server(%ld): Connection closing with error...\n", m_socket_id);
  // just in case, make sure the callbacks are no longer active

  // remove the send callback
  socket->SetSendCallback (MakeNullCallback<void, Ptr<Socket>, uint32_t > ());
  // remove the recv callback
  socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());

  this->m_finished_callback(this->m_socket_id);
}


// GetFileSize either from m_fileSizes map or from disk
long HttpServerFakeClientSocket::GetFileSize(std::string filename)
{
  // check if is already in m_fileSizes
  if (m_fileSizes.find(filename) != m_fileSizes.end())
  {
    return m_fileSizes[filename];
  }
  // else: query disk for file size

  struct stat stat_buf;
  int rc = stat(filename.c_str(), &stat_buf);

  if (rc == 0)
  {
    m_fileSizes[filename] = stat_buf.st_size;
    return stat_buf.st_size;
  }
  // else: file not found
  fprintf(stderr, "Server(%ld) ERROR: File not found: '%s'\n", m_socket_id, filename.c_str());
  return -1;
}



void
HttpServerFakeClientSocket::LogCwndChange(uint32_t oldCwnd, uint32_t newCwnd)
{
  fprintf(stderr, "Server(%ld): Cwnd Changed %d -> %d\n", m_socket_id, oldCwnd, newCwnd);
}


void
HttpServerFakeClientSocket::LogStateChange(const ns3::TcpSocket::TcpStates_t old_state, const ns3::TcpSocket::TcpStates_t new_state)
{
  fprintf(stderr, "Server(%ld): Socket State Change %s -> %s\n", m_socket_id, ns3::TcpSocket::TcpStateName[old_state], ns3::TcpSocket::TcpStateName[new_state]);
}

void
HttpServerFakeClientSocket::FinishedIncomingData(Ptr<Socket> socket, Address from, std::string data)
{
  fprintf(stderr, "Server(%ld)::FinishedIncomingData(socket,data=str(%ld))\n", m_socket_id, data.length());
  // now parse this request (TODO) and reply
  std::string filename = m_content_dir  + ParseHTTPHeader(data);

  fprintf(stderr, "Server(%ld): Opening '%s'\n", m_socket_id, filename.c_str());

  long filesize = GetFileSize(filename);

  m_is_virtual_file = false;

  if (filesize == -1)
  {
    fprintf(stderr, "Server(%ld): Error, '%s' not found!\n", m_socket_id, filename.c_str());
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

    //fprintf(stderr, "Replying with header:\n%s\n", replySS.str().c_str());

    std::string replyString = replySS.str();
    uint8_t* buffer = (uint8_t*)replyString.c_str();
    AddBytesToTransmit(buffer,replyString.length());

    // now append the virtual payload data
    uint8_t tmp[4096];


    if (std::find(m_virtualFiles.begin(), m_virtualFiles.end(), filename) != m_virtualFiles.end())
    {
      // handle virtual payload
      // fill tmp with some random data
      fprintf(stderr, "Server(%ld): Generating virtual payload of size %ld ...\n", m_socket_id, filesize);


      this->m_totalBytesToTx += filesize;
      this->m_is_virtual_file = true;

      /*
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
      } */
    } else
    {
      fprintf(stderr, "Server(%ld): Opening file on disk with size %ld ...\n", m_socket_id, filesize);
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

void
HttpServerFakeClientSocket::HandleReadyToTransmit(Ptr<Socket> socket, uint32_t txSize)
{
  //fprintf(stderr, "Server(%ld): HandleReadyToTransmit(txSize=%d)\n", m_socket_id, txSize);

  if (m_totalBytesToTx == 0) // do nothing
  {
    fprintf(stderr, "Server(%ld)::HandleReadyToTransmit: Nothing to transmit (yet)...\n", m_socket_id);
    return;
  }
  if (m_currentBytesTx >= m_totalBytesToTx && m_totalBytesToTx > 0)
  {
    // already sent everything, check if we need to "close" the socket and disband this object, or if we keep it alive
    if (!m_keep_alive)
    {
      if (!m_is_shutdown)
      {
        fprintf(stderr, "Server(%ld)::HandleReadyToTransmit: Sent %d bytes, now shutting down client socket (socket->close())\n", m_socket_id, m_currentBytesTx);

        // Request this socket to close
        socket->Close();
        // remove the send callback
        socket->SetSendCallback (MakeNullCallback<void, Ptr<Socket>, uint32_t > ());
        // remove the recv callback
        socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());

        // reset the header data from recvstring
        m_activeRecvString = "";

        // we already finished sending, so we can clear the buffer for the sake of saving memory
        this->m_bytesToTransmit.clear();
        std::vector<uint8_t>().swap( this->m_bytesToTransmit ); // explicitly clear the buffer
        m_is_shutdown = true; // make sure to set that flag to true, so that we do not call this stuff again
      }
    } else {
      // keeping connection alive

      // reset the header data from recvstring
      m_activeRecvString = "";

      // we already finished sending, so we can clear the buffer for the sake of saving memory
      this->m_bytesToTransmit.clear();
      std::vector<uint8_t>().swap( this->m_bytesToTransmit ); // explicitly clear the buffer
      m_is_shutdown = true;
    }
    return;
  }
  //fprintf(stderr, "Server(%ld)::HandleReadyToTransmit(socket,txSize=%u)\n", m_socket_id, txSize);


  // get txSize bytes from m_bytesToTransmit, starting at byte m_currentBytesTx


  while (m_currentBytesTx < m_totalBytesToTx && socket->GetTxAvailable () > 0)
  {
    uint32_t remainingBytes = m_totalBytesToTx - m_currentBytesTx;

    if (remainingBytes > 2860)
      remainingBytes = 2860;

    remainingBytes = std::min(remainingBytes, socket->GetTxAvailable ());

    Ptr<Packet> replyPacket;

    if (!m_is_virtual_file)
    {
      uint8_t* buffer = (uint8_t*) &((this->m_bytesToTransmit)[m_currentBytesTx]);
      replyPacket = Create<Packet> (buffer, remainingBytes);
    } else
    {
      if (m_currentBytesTx == 0) // reply with header
      {
        uint8_t* buffer = (uint8_t*) &((this->m_bytesToTransmit)[m_currentBytesTx]);
        replyPacket = Create<Packet> (buffer, this->m_bytesToTransmit.size());
      } else {
        // create a virtual reply packet
        uint8_t* buffer = (uint8_t*)malloc(remainingBytes);

        replyPacket = Create<Packet> (buffer, remainingBytes);
        free(buffer);
      }
    }



    int amountSent = socket->Send (replyPacket);
    if (amountSent <= 0)
    {
      fprintf(stderr, "Server(%ld): failed to transmit %d bytes, waiting for next transmit...\n", m_socket_id, remainingBytes);
      // we will be called again, when new TX space becomes available;
      return;
    }

    m_currentBytesTx += amountSent;

    //fprintf(stderr, "Server(%ld)::HandleReadyToTransmit - Transmitted %d bytes, %u remaining\n", m_socket_id, amountSent, m_totalBytesToTx - m_currentBytesTx);
  }
}


void HttpServerFakeClientSocket::AddBytesToTransmit(const uint8_t* buffer, uint32_t size)
{
  std::copy(buffer, buffer+size, std::back_inserter(this->m_bytesToTransmit));
  this->m_totalBytesToTx += size;
}

};
