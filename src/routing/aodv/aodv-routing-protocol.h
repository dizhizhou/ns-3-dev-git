/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 IITP RAS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Based on 
 *      NS-2 AODV model developed by the CMU/MONARCH group and optimized and
 *      tuned by Samir Das and Mahesh Marina, University of Cincinnati;
 * 
 *      AODV-UU implementation by Erik Nordström of Uppsala University
 *      http://core.it.uu.se/core/index.php/AODV-UU
 *
 * Authors: Elena Borovkova <borovkovaes@iitp.ru>
 *          Pavel Boyko <boyko@iitp.ru>
 */
#ifndef AODVROUTINGPROTOCOL_H_
#define AODVROUTINGPROTOCOL_H_

#include "aodv-rtable.h"
#include "aodv-rqueue.h"
#include "aodv-packet.h"
#include "id-cache.h"
#include "aodv-neighbor.h"

#include "src/internet-stack/ipv4-l3-protocol.h"

#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/timer.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-routing-protocol.h"
#include <map>

namespace ns3
{
namespace aodv
{

/**
 * \ingroup aodv
 * \brief AODV routing protocol
 */
class RoutingProtocol : public Ipv4RoutingProtocol
{
public:
  static TypeId GetTypeId (void);
  static const uint32_t AODV_PORT;

  RoutingProtocol();
  virtual ~RoutingProtocol();
  virtual void DoDispose ();
  
  ///\name From Ipv4RoutingProtocol
  //\{
  Ptr<Ipv4Route> RouteOutput (Ptr<Packet> p, const Ipv4Header &header, uint32_t oif, Socket::SocketErrno &sockerr);
  bool RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                           UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                           LocalDeliverCallback lcb, ErrorCallback ecb);  
  virtual void NotifyInterfaceUp (uint32_t interface);
  virtual void NotifyInterfaceDown (uint32_t interface);
  virtual void NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address);
  virtual void NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address);
  virtual void SetIpv4 (Ptr<Ipv4> ipv4);
  //\}
  
  ///\name Handle protocol parameters
  //\{
  bool GetDesinationOnlyFlag () const { return DestinationOnly; }
  void SetDesinationOnlyFlag (bool f) { DestinationOnly = f; }
  bool GetGratuitousReplyFlag () const { return GratuitousReply; }
  void SetGratuitousReplyFlag (bool f) { GratuitousReply = f; }
  void SetExpandingRingSearchEnable (bool f) { EnableExpandingRingSearch = f; }
  bool GetExpandingRingSearchEnable () const { return EnableExpandingRingSearch; }
  void SetHelloEnable (bool f) { EnableHello = f; }
  bool GetHelloEnable () const { return EnableHello; }
  void SetLocalRepairEnable (bool f) { EnableLocalRepair = f; }
  bool GetLocalRepairEnable () const { return EnableLocalRepair; }
  //\}
private:
  ///\name Protocol parameters. TODO document
  //\{
  uint32_t RreqRetries;             ///< Maximum number of retransmissions of RREQ with TTL = NetDiameter to discover a route
  Time ActiveRouteTimeout;          ///< Minimal lifetime for active route.
  uint32_t NetDiameter;             ///< Net diameter measures the maximum possible number of hops between two nodes in the network
  /**
   *  NodeTraversalTime is a conservative estimate of the average one hop traversal time for packets
   *  and should include queuing delays, interrupt processing times and transfer times.
   */
  Time NodeTraversalTime;
  Time NetTraversalTime;             ///< Estimate of the average net traversal time.
  Time PathDiscoveryTime;            ///< Estimate of maximum time needed to find route in network
  Time MyRouteTimeout;               ///< Value of lifetime field in RREP generating by this node
  /**
   * Every HelloInterval the node checks whether it has sent a broadcast  within the last HelloInterval.
   * If it has not, it MAY broadcast a  Hello message
   */
  Time HelloInterval;
  uint32_t AllowedHelloLoss;         ///< Number of hello messages which may be loss for valid link
  /**
   * DeletePeriod is intended to provide an upper bound on the time for which an upstream node A
   * can have a neighbor B as an active next hop for destination D, while B has invalidated the route to D.
   */
  Time DeletePeriod;
  Time NextHopWait;                  ///< Period of our waiting for the neighbour's RREP_ACK
  uint16_t TtlStart;                 ///< Initial value of TTL in RREQ  when use an expanding ring search
  uint16_t TtlIncrement;             ///< Increment value of RREQ TTL when use an expanding ring search
  uint16_t TtlThreshold;             ///< Threshold, beyond which TTL = NetDiameter is used for each attempt in RREQ
  uint16_t  MaxRepairTtl;            ///< Maximum distance in hops between intermediate node and destination node when local repair still may be applied.
  uint16_t LocalAddTtl;              ///< Value used in calculation RREQ TTL when use local repair
  /**
   * The TimeoutBuffer is configurable.  Its purpose is to provide a buffer for the timeout so that if the RREP is delayed
   * due to congestion, a timeout is less likely to occur while the RREP is still en route back to the source.
   */
  uint16_t TimeoutBuffer;
  Time BlackListTimeout;             ///< Time for which the node is put into the blacklist
  uint32_t MaxQueueLen;              ///< The maximum number of packets that we allow a routing protocol to buffer.
  Time MaxQueueTime;                 ///< The maximum period of time that a routing protocol is allowed to buffer a packet for.
  bool DestinationOnly;              ///< Indicates only the destination may respond to this RREQ.
  bool GratuitousReply;              ///< Indicates whether a gratuitous RREP should be unicast to the node originated route discovery.
  bool EnableExpandingRingSearch;    ///< Indicates whether a expanding ring search enable
  bool EnableHello;                  ///< Indicates whether a hello messages enable
  bool EnableLocalRepair;            ///< Indicates whether a local repair enable
  //\}

  /// IP protocol
  Ptr<Ipv4> m_ipv4;
  /// Raw socket per each IP interface, map socket -> iface address (IP + mask)
  std::map< Ptr<Socket>, Ipv4InterfaceAddress > m_socketAddresses;

  /// Routing table
  RoutingTable m_routingTable;
  /// A "drop-front" queue used by the routing layer to buffer packets to which it does not have a route.
  RequestQueue m_queue;
  /// Broadcast ID
  uint32_t m_requestId;
  /// Request sequence number
  uint32_t m_seqNo;
  /// Handle duplicated packets
  IdCache m_idCache;
  /// Handle neighbors
  Neighbors m_nb;
  /// Address of the destination, which currently repaired.
  Ipv4Address m_repairedDst;

  /// Unicast callback for own packets
  UnicastForwardCallback m_scb;
  /// Error callback for own packets
  ErrorCallback m_ecb;

private:
  /// Start protocol operation
  void Start ();
  /// Start local route repair procedure
  void LocalRouteRepair (Ipv4Address dst, Ipv4Address origin);
  /**
   * If route exists and valid, forward packet.
   * If route exists and down try to repair route if following conditions is true
   *    1. Using local route repair technique enable
   *    2. The destination is no farther than MAX_REPAIR_TTL hops away.
   * During local repair data packets SHOULD be buffered.
   * \return true if node forward packet or try to repair route.
   */
  bool Forwarding (Ptr<const Packet> p, const Ipv4Header & header, UnicastForwardCallback ucb, ErrorCallback ecb);
  /**
  * To reduce congestion in a network, repeated attempts by a source node at route discovery
  * for a single destination MUST utilize a binary exponential backoff.
  */
  void ScheduleRreqRetry (Ipv4Address dst,  uint16_t ttl);
  /**
   * Update route lifetime.
   * \param addr - destination address
   * \return true if route to destination address addr exist
   */
  bool UpdateRouteLifeTime(Ipv4Address addr, Time lifetime);
  /// Update neighbor record. \param receiver is supposed to be my interface
  void UpdateRouteToNeighbor (Ipv4Address sender, Ipv4Address receiver);
  /// Check that packet is send from own interface
  bool IsMyOwnAddress(Ipv4Address src);
  /// Find socket with local interface address iface
  Ptr<Socket> FindSocketWithInterfaceAddress (Ipv4InterfaceAddress iface) const;
  /// Process hello message
  void ProcessHello(RrepHeader const & rrepHeader, Ipv4Address receiverIfaceAddr);
  
  ///\name Receive control packets
  //\{
  /// Receive and process control packet
  void RecvAodv (Ptr<Socket> socket);
  /// Receive RREQ
  void RecvRequest (Ptr<Packet> p, Ipv4Address receiver, Ipv4Address src, Ipv4Header ipv4Header);
  /// Receive RREP
  void RecvReply (Ptr<Packet> p, Ipv4Address my ,Ipv4Address src, Ipv4Header ipv4Header);
  /// Receive RREP_ACK
  void RecvReplyAck(Ipv4Address neighbor);
  /// Receive RERR from node with address src
  void RecvError (Ptr<Packet> p, Ipv4Address src);
  //\}
  
  ///\name Send
  //\{
  /// Forward packet from route request queue
  void SendPacketFromQueue(Ipv4Address dst, Ptr<Ipv4Route> route);
  /// Aux. send helper
  void Send(Ptr<Ipv4Route>, Ptr<const Packet>, const Ipv4Header &);
  /// Send hello
  void SendHello ();
  /// Send RREQ
  void SendRequest (Ipv4Address dst, uint16_t ttl);
  /// Send RREP
  void SendReply (RreqHeader const & rreqHeader, RoutingTableEntry const & toOrigin);
  /** Send RREP by intermediate node
   * \param toDst routing table entry to destination
   * \param toOrigin routing table entry to originator
   * \param gratRep indicates whether a gratuitous RREP should be unicast to destination
   */
  void SendReplyByIntermediateNode (RoutingTableEntry & toDst, RoutingTableEntry & toOrigin, bool gratRep);
  /// Send RREP_ACK
  void SendReplyAck (Ipv4Address neighbor);
  /// Initiate RERR
  void SendRerrWhenBreaksLinkToNextHop (Ipv4Address nextHop);
  void SendRerr (Ipv4Address dst, bool noDelete);
  /// Forward RERR
  void SendRerrMessage(Ptr<Packet> packet,  std::vector<Ipv4Address> precursors);
  /**
  * Add UDP, IP headers to packet and send it via raw socket
  */
  void SendPacketViaRawSocket (Ptr<Packet> packet, std::pair<Ptr<Socket> , Ipv4InterfaceAddress> socketAddress, Ipv4Address dst, uint16_t ttl, uint16_t id);
  //\}
  
  /// Notify that packet is dropped for some reason 
  void Drop(Ptr<const Packet>, const Ipv4Header &, Socket::SocketErrno);

  ///\name Timers. TODO comment each one
  //\{
  Timer htimer; // TODO independent hello timers for all interfaces
  void HelloTimerExpire ();
  Timer lrtimer;
  void LocalRepairTimerExpire ();
  std::map<Ipv4Address, Timer> m_addressReqTimer;
  void RouteRequestTimerExpire(Ipv4Address dst, uint16_t lastTtl);
  void AckTimerExpire (Ipv4Address neighbor,  Time blacklistTimeout);
  //\}
};

}
}
#endif /* AODVROUTINGPROTOCOL_H_ */