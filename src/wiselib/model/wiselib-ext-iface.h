/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

// Author: Dizhi Zhou (dizhi.zhou@gmail.com)

#ifndef WISELIB_EXT_IFACE_H
#define WISELIB_EXT_IFACE_H

#include <stdint.h>
#include <iostream>
#include "ns3/simulator.h"
#include "ns3/event-id.h"

namespace wiselib {

class WiselibExtIface
{
public:

  typedef int node_id_t;
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

  void send_wiselib_message( node_id_t id, size_t len, block_data_t *data );

  node_id_t id ();

  template<void (*TMethod)( node_id_t, size_t, block_data_t* )>
  bool reg_recv_callback() 
    {
      return true;
    }


private:
  typedef ns3::EventId Ns3EventId;
  typedef ns3::Time Ns3Time;

  Ns3EventId m_timerFacetEvent;

};

}

#endif /* WISELIB_EXT_IFACE_H */

