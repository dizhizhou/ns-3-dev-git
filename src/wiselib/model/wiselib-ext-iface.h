/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

// Author: Dizhi Zhou (dizhi.zhou@gmail.com)

#ifndef WISELIB_EXT_IFACE_H
#define WISELIB_EXT_IFACE_H

#include <stdint.h>
#include <iostream>
#include <map>
#include <vector>
#include "ns3/simulator.h"
#include "ns3/event-id.h"

#include "ns3/node-container.h"
#include "ns3/ptr.h"
#include "ns3/socket.h"
#include "ns3/node.h"
#include "ns3/net-device-container.h"
#include "ns3/address.h"
#include "ns3/packet.h"
#include "ns3/event-impl.h"
#include "ns3/type-traits.h"
#include "ns3/make-event.h"
#include "ns3/wifi-helper.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/nqos-wifi-mac-helper.h"
#include "ns3/vector.h"

namespace wiselib {

class EventImplExt : public ns3::EventImpl
{
public:
   EventImplExt () { }
   virtual ~EventImplExt () { }

   virtual void RecvCallback (uint32_t , size_t, unsigned char*) { };
};

template <typename MEM, typename OBJ>
EventImplExt * MakeRecvCallback (MEM mem_ptr, OBJ obj)
{
  // zero argument version
  class EventMemberImpl0 : public EventImplExt
  {
public:
    EventMemberImpl0 (OBJ obj, MEM function)
      : m_obj (obj),
        m_function (function)
    {
    }
    virtual ~EventMemberImpl0 ()
    {
    }
  
    // call use-defined callback
    virtual void RecvCallback (uint32_t from, size_t len, unsigned char* data)
    {
      (ns3::EventMemberImplObjTraits<OBJ>::GetReference (m_obj).*m_function)(from, len, data);
    }

private:
    
    virtual void Notify (void)
    {
    }
    
    OBJ m_obj;
    MEM m_function;
  } *ev = new EventMemberImpl0 (obj, mem_ptr);
  return ev;
}


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

  // register receive callback method for all sink nodes
  template<typename T, void (T::*TMethod)( node_id_t, size_t, block_data_t* )>
  bool RegRecvCallback( T *obj_pnt, node_id_t local ) 
    {
      ns3::Address addr = nodes.Get (local)->GetDevice (0)->GetAddress ();
      std::map<ns3::Address, ns3::Ptr<ns3::NetDevice> >::iterator it = addDevMap.end (); 
      it = addDevMap.find (addr);
      if (it != addDevMap.end ()) 
        {
          // regeister NS-3 callback
          it->second->SetReceiveCallback (ns3::MakeCallback 
                                  (&WiselibExtIface::DoRegRecvCallback, this));

          // store function and member which will be called in NS-3 callback
          EventImplExt *event = MakeRecvCallback(TMethod, obj_pnt);
          m_recvCallBackMap.insert(std::pair<node_id_t,EventImplExt*>(local, event));
        }

      return true;
    }

  bool DoRegRecvCallback (ns3::Ptr<ns3::NetDevice> device, ns3::Ptr<const ns3::Packet> packet,
                          uint16_t protocol, const ns3::Address &from) 
    {
      uint8_t buffer[packet->GetSize ()]; 
      packet->CopyData (buffer, sizeof(buffer)); 
      // call ns3::Node::ReceiveFromDevice method ?
      std::map<ns3::Address, ns3::Ptr<ns3::NetDevice> >::iterator it = addDevMap.end (); 
      it = addDevMap.find (device->GetAddress ());
      if (it != addDevMap.end ()) 
        {
          node_id_t sendId = (addDevMap.find (from))->second->GetNode ()->GetId ();
          node_id_t recvId = it->second->GetNode ()->GetId ();

          std::map <node_id_t, EventImplExt*>::iterator itMap = m_recvCallBackMap.end ();
          itMap = m_recvCallBackMap.find (recvId);       
          if (itMap != m_recvCallBackMap.end ())
            {
              itMap->second->RecvCallback (sendId, sizeof(buffer), buffer);
            }
          else
            std::cout << "Unknow receiver node id " << std::endl;
        }
      else 
        std::cout << "Unknow node id " << std::endl;
      return true;
    }

  // define default phy (802.11b), mac (adhoc) and wifi helper instance
  void Init ();

  // create 1 node and install phy, mac on it
  // return the node id to user
  node_id_t EnableRadio ();

  // send mac layer packet
  void SendWiselibMessage( node_id_t dest, size_t len, block_data_t *data, node_id_t local );

  // clock facet support
  double GetNs3Time ();

  // position facet support
  double GetPositionX (node_id_t id);
  double GetPositionY (node_id_t id);
  double GetPositionZ (node_id_t id);
  // set position support in position facet
  void SetPosition (double x, double y, double z, node_id_t id);


  // distance facet support
  double GetDistance (node_id_t src, node_id_t dst);

private:
  typedef ns3::EventId Ns3EventId;
  typedef ns3::Time Ns3Time;

  Ns3EventId m_timerFacetEvent;

  // wifi parameters
  ns3::NodeContainer nodes;                        // note: we use the node index in index as the node id
  ns3::YansWifiPhyHelper wifiPhy;
  ns3::NqosWifiMacHelper wifiMac;
  ns3::WifiHelper wifi;

  std::map<ns3::Address, ns3::Ptr<ns3::NetDevice> > addDevMap;  // Address --> NetDevice --> Node --> node id
  std::map <node_id_t, EventImplExt*> m_recvCallBackMap; // store the mem and obj of user-defined receive callback
  std::map <node_id_t, ns3::Ptr<ns3::Node> > nodeMap; // store the node id and node object in ns3
};

}

#endif /* WISELIB_EXT_IFACE_H */

