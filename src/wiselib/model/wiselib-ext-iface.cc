/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

// Author: Dizhi Zhou (dizhi.zhou@gmail.com)

#include "wiselib-ext-iface.h"
#include "ns3/log.h"
#include <iostream>
#include <cstdarg>
#include <cstdio>

NS_LOG_COMPONENT_DEFINE ("WiselibExtIface");

namespace wiselib {

WiselibExtIface::WiselibExtIface ()
{
}

WiselibExtIface::~WiselibExtIface ()
{
}

void
WiselibExtIface::Debug (const char *msg)
{
  LogComponentEnable ("WiselibExtIface", ns3::LOG_DEBUG);
  NS_LOG_DEBUG (msg);
}

void 
WiselibExtIface::send_wiselib_message( node_id_t id, size_t len, block_data_t *data )
{
}

WiselibExtIface::node_id_t 
WiselibExtIface::id ()
{
  return 0;
} 
}

