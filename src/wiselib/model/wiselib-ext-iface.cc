/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

// Author: Dizhi Zhou (dizhi.zhou@gmail.com)

#include <iostream>
#include <cstdarg>
#include <cstdio>

#include "wiselib-ext-iface.h"
#include "ns3/log.h"
#include "ns3/config.h"
#include "ns3/string.h"
#include "ns3/double.h"
#include "ns3/wifi-helper.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/nqos-wifi-mac-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/packet.h"
#include "ns3/node-container.h"

NS_LOG_COMPONENT_DEFINE ("WiselibExtIface");

namespace wiselib {

WiselibExtIface::WiselibExtIface ()
{
  Init ();
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
WiselibExtIface::SendWiselibMessage( node_id_t dest, size_t len, block_data_t *data, node_id_t local )
{
  // TBD: use MAC48 broadcast address
  uint32_t addr = dest;
  ns3::Ipv4Address ipv4Addr(addr);

  if (ipv4Addr == ns3::Ipv4Address::GetBroadcast ())
    {
      // broadcast
      ns3::Address addr = nodes.Get (local)->GetDevice (0)->GetAddress ();
      std::map<ns3::Address, ns3::Ptr<ns3::NetDevice> >::iterator it = addDevMap.end (); 
      it = addDevMap.find (addr);
      if (it != addDevMap.end ()) 
        {
          ns3::Ptr<ns3::Packet> packet = ns3::Create<ns3::Packet> (data, len);
          it->second->Send (packet, it->second->GetBroadcast (), 0);
        }
    }
  else
    {
      // unicast
      ns3::Address addr = nodes.Get (local)->GetDevice (0)->GetAddress ();
      std::map<ns3::Address, ns3::Ptr<ns3::NetDevice> >::iterator it = addDevMap.end (); 
      it = addDevMap.find (addr);
      if (it != addDevMap.end ()) 
        {
          // find dest node's mac address
          ns3::Address dst = nodes.Get (dest)->GetDevice (0)->GetAddress ();
          ns3::Ptr<ns3::Packet> packet = ns3::Create<ns3::Packet> (data, len);
          it->second->Send (packet, dst, 0);
        }

    }
}

WiselibExtIface::node_id_t
WiselibExtIface::EnableRadio ()
{
  ns3::NodeContainer node;
  node.Create (1); 
  nodes.Add (node.Get (0)); 
  ns3::NetDeviceContainer device = wifi.Install (wifiPhy, wifiMac, node);

  ns3::MobilityHelper mobility;
  ns3::Ptr<ns3::ListPositionAllocator> positionAlloc = ns3::CreateObject<ns3::ListPositionAllocator> ();
  positionAlloc->Add (ns3::Vector (0.0, 0.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (node);

  addDevMap.insert (std::pair<ns3::Address, ns3::Ptr<ns3::NetDevice> >
                        (device.Get (0)->GetAddress (), device.Get (0)));      

  return nodes.Get (nodes.GetN () - 1)->GetId ();
}


void
WiselibExtIface::Init ()
{
  std::string phyMode ("DsssRate11Mbps");	
  double rss = -80;  // -dBm

  // disable fragmentation for frames below 2200 bytes
  ns3::Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", ns3::StringValue ("2200"));
  // turn off RTS/CTS for frames below 2200 bytes
  ns3::Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", ns3::StringValue ("2200"));
  // Fix non-unicast data rate to be the same as that of unicast
  ns3::Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", 
                      ns3::StringValue (phyMode));

  // The below set of helpers will help us to put together the wifi NICs we want
  wifi.SetStandard (ns3::WIFI_PHY_STANDARD_80211b);

  wifiPhy =  ns3::YansWifiPhyHelper::Default ();
  // This is one parameter that matters when using FixedRssLossModel
  // set it to zero; otherwise, gain will be added
  wifiPhy.Set ("RxGain", ns3::DoubleValue (0) ); 
  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  wifiPhy.SetPcapDataLinkType (ns3::YansWifiPhyHelper::DLT_IEEE802_11_RADIO); 

  ns3::YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  // The below FixedRssLossModel will cause the rss to be fixed regardless
  // of the distance between the two stations, and the transmit power
  wifiChannel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss",ns3::DoubleValue (rss));
  wifiPhy.SetChannel (wifiChannel.Create ());

  // Add a non-QoS upper mac, and disable rate control
  wifiMac = ns3::NqosWifiMacHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",ns3::StringValue (phyMode),
                                "ControlMode",ns3::StringValue (phyMode));

  // Set it to adhoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac");
}
  

}

