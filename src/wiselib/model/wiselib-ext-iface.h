/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

// Author: Dizhi Zhou (dizhi.zhou@gmail.com)

#ifndef WISELIB_EXT_IFACE_H
#define WISELIB_EXT_IFACE_H

#include <stdint.h>
#include <iostream>
#include "ns3/simulator.h"
#include "ns3/event-id.h"

#include "ns3/node-container.h"
#include "ns3/ptr.h"
#include "ns3/socket.h"
#include "ns3/node.h"
#include "ns3/net-device-container.h"
#include "ns3/address.h"
#include "ns3/packet.h"

namespace wiselib {

class WiselibExtIface
{
public:

  typedef uint32_t node_id_t;    // we use unsigned because NS-3 does not have node id smaller than zero
  typedef unsigned char block_data_t;
  typedef long size_t;
  typedef uint8_t message_id_t;


  WiselibExtIface ();

  virtual ~WiselibExtIface ();

  void Debug (const char *msg);

  template<typename T, void (T::*TMethod)(void*)>
  bool SetTimeout( uint32_t millis, T *obj_pnt, void *userdata )
    {

      Ns3Time delay = ns3::MilliSeconds(millis);

      // Note:
      //   1, user does not define the typename for Simulator::Schedule. In this case, the compiler will deduct the typename
      //   2, delegate in Wiselib is not used here. In NS-3, Simulator::Schedule provides a similar solution to realize delegate.
      //      more details about delegate in NS-3 can be found in src/core/model/make-event.h
      m_timerFacetEvent = ns3::Simulator::Schedule (delay, TMethod, obj_pnt, userdata);
      // Q: must I cancel the event when it is expired? 

      //(obj_pnt->*TMethod) (userdata); // call member function TMethod now
      return true;
    }


  // TBD: node id function is disabled in the current simulation
  node_id_t id (); 

  // Function: register receive callback method for all sink nodes
  // TBD: set receive callback method for the sink node with node_id_t
  template<typename T, void (T::*TMethod)( node_id_t, size_t, block_data_t* )>
  bool regRecvCallback( T *obj_pnt ) 
    {
      for (uint16_t i = 0; i < recvSockets.size (); i++)
        {
          // we cannot add user-defined callbacks in here due to the fixed callback format in ns-3
          recvSockets.at (i)->SetRecvCallback (ns3::MakeCallback 
                                  (&WiselibExtIface::DoRegRecvCallback, this));
          // TBD: save user-defined callbacks: (socket -- TMethod, obj_pnt)
        }
      return true;
    }

  void DoRegRecvCallback (ns3::Ptr<ns3::Socket> socket) 
    {
      // TBD: call user user-defined callbacks.
      ns3::Address from;
      ns3::Ptr<ns3::Packet> pkt;
      while (pkt = socket->RecvFrom (from))
        {
          uint8_t buffer[pkt->GetSize ()]; 
          pkt->CopyData (buffer, sizeof(buffer)); 
          std::cout << ns3::Simulator::Now ().GetSeconds () << " " << buffer << std::endl;
 
          // reg_call_back (node_id, sizeof (buffer), buffer)
        }
    }


  // Function: 
  //    1. define default phy (802.11b), mac (adhoc) and ip (ipv4) layer of nodes
  //    2. create 3 default nodes. One is sender and other two nodes are sinks.
  //    3. two sink nodes have the same distance(5 meters) with the source node
  void initRadio ();

  // use UDP packet to send data
  void sendWiselibMessage( node_id_t id, size_t len, block_data_t *data );

private:
  typedef ns3::EventId Ns3EventId;
  typedef ns3::Time Ns3Time;

  Ns3EventId m_timerFacetEvent;

  ns3::NodeContainer nodes;                        // note: we use the node index in index as the node id
  std::vector<ns3::Ptr<ns3::Socket> > recvSockets; // sockets on sink node, default value = UDP socket
  std::vector<ns3::Ptr<ns3::Socket> > sendSockets; // sockets on source node, default value = UDP socket
};

}

#endif /* WISELIB_EXT_IFACE_H */

