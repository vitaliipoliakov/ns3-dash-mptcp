/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
//
// Copyright (c) 2015 Christian Kreuzberger, Alpen-Adria-Universitaet Klagenfurt
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation;
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Author: Christian Kreuzberger <christian.kreuzberger@itec.aau.at>
//

// ns3 - HTTP Client Application class


#ifndef HTTP_CLIENT_APPLICATION_H
#define HTTP_CLIENT_APPLICATION_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/traced-callback.h"
#include "ns3/tcp-socket.h"



#define CRLF "\r\n"


namespace ns3 {



class Socket;
class Packet;

/**
 * \ingroup udpecho
 * \brief A Udp Echo client
 *
 * Every packet sent should be returned by the server and received here.
 */
class HttpClientApplication : public Application
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  HttpClientApplication ();

  virtual ~HttpClientApplication ();

  /**
   * \brief set the remote address and port
   * \param ip remote IPv4 address
   * \param port remote port
   */
  void SetRemote (Ipv4Address ip, uint16_t port);
  /**
   * \brief set the remote address and port
   * \param ip remote IPv6 address
   * \param port remote port
   */
  void SetRemote (Ipv6Address ip, uint16_t port);
  /**
   * \brief set the remote address and port
   * \param ip remote IP address
   * \param port remote port
   */
  void SetRemote (Address ip, uint16_t port);



  /**
   * \brief Forces closing the socket if KeepAlive was set
   */
  void ForceCloseSocket();


  /**
   * \brief Cancel the current download by closing the socket
  */

  void CancelDownload ();


protected:
  virtual void DoDispose (void);

  bool do_cancel_socket;
  bool m_is_first_packet;
  bool m_has_parsed_response_header;

  bool m_finished_download;

  unsigned int requested_content_length;



  double lastDownloadBitrate;

  uint32_t node_id;

  bool m_keepAlive;


protected: // callbacks/traces
  TracedCallback<Ptr<ns3::Application> /* app */, std::string /* interestName */> m_downloadStartedTrace;
  TracedCallback<Ptr<ns3::Application> /* app */, std::string /* interestName */,
            long /*fileSize*/> m_headerReceivedTrace;
  TracedCallback<Ptr<ns3::Application> /* app */, std::string /* interestName */,
            double /* downloadSpeedInBytesPerSecond */, long /*milliSeconds */> m_downloadFinishedTrace;
  TracedCallback<Ptr<ns3::Application> /* app */, std::string /* interestName */,
            unsigned int /* bytes_recv */> m_currentStatsTrace;


  void ConnectionComplete (Ptr<Socket> socket);
  void ConnectionFailed (Ptr<Socket> socket);
  void ConnectionClosedNormal (Ptr<Socket> socket);
  void ConnectionClosedError (Ptr<Socket> socket);

  void TryEstablishConnection();

  virtual void OnFileReceived(unsigned status, unsigned length);


  void ReportStats();


  uint32_t ParseResponseHeader (const uint8_t* buffer, size_t len, int* statusCode, unsigned int* contentLength);

  void LogStateChange(const  ns3::TcpSocket::TcpStates_t old_state, const  ns3::TcpSocket::TcpStates_t new_state);

  void LogCwndChange(uint32_t oldCwnd, uint32_t newCwnd);

  virtual void StartApplication (void);
  virtual void StopApplication (void);


  std::string m_fileToRequest;
  std::string m_hostName; //!< The hostname of the destiatnion server
  std::string m_outFile;

  bool m_active;

  uint32_t cur_cwnd;

private:

  uint8_t* _tmpbuffer;

  /**
   * \brief Callback from Socket when ready to send a packet
   */
  virtual void OnReadySend (Ptr<Socket> localSocket, uint32_t txSpace);


  /**
   * \brief Sending an actual packet
   */
  virtual void DoSendGetRequest (Ptr<Socket> localSocket, uint32_t txSpace);

  /**
   * \brief Handle a packet reception.
   *
   * This function is called by lower layers.
   *
   * \param socket the socket the packet was received to.
   */
  void HandleRead (Ptr<Socket> socket);

  Time m_interval; //!< Packet inter-send time
  uint32_t m_size; //!< Size of the sent packet

  uint32_t m_sent; //!< Counter for sent packets
  Ptr<Socket> m_socket; //!< Socket
  Address m_peerAddress; //!< Remote peer address
  uint16_t m_peerPort; //!< Remote peer port
  EventId m_sendEvent; //!< Event to send the next packet
  EventId m_reportStatsEvent; //!< Event to report statistics


  uint32_t m_bytesRecv; //!< Number of bytes received
  uint32_t m_bytesSent; //!< Number of bytes sent

  uint32_t m_lastStatsReportedBytesRecv;
  uint32_t m_lastStatsReportedBytesSent;

  bool m_sentGetRequest; //!< Indicates whether a GET request has been sent yet or not


  /// Callbacks for tracing the packet Tx events
  TracedCallback<Ptr<const Packet> > m_txTrace;



  int64_t _start_time;
  int64_t _finished_time;

  uint32_t m_tried_connecting;
  uint32_t m_success_connecting;
  uint32_t m_failed_connecting;
};

} // namespace ns3

#endif /* HTTP_CLIENT_APPLICATION_H */
