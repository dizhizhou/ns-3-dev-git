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
WiselibExtIface::sendWiselibMessage( node_id_t id, size_t len, block_data_t *data )
{
  if (id)
    {
      uint32_t addr = id;
      ns3::Ipv4Address ipv4Addr(addr);

      // this is a broadcast message
      if ( ipv4Addr == ns3::Ipv4Address::GetBroadcast () )
        {

          // TBD: send packets to MAC layer directly
          for (uint16_t i = 0; i < recvSockets.size (); i++)
            {
              ns3::InetSocketAddress local = ns3::InetSocketAddress (ns3::Ipv4Address::GetAny (), 80);
              recvSockets.at (i)->Bind (local);
            }
          for (uint16_t i = 0; i < sendSockets.size (); i++)
            {
              ns3::InetSocketAddress remote = ns3::InetSocketAddress (ns3::Ipv4Address::GetBroadcast (), 80);
              sendSockets.at (i)->SetAllowBroadcast (true);
              sendSockets.at (i)->Connect (remote);

              // create packet
              // TBD: convert unsign char to uint8_t?
              ns3::Ptr<ns3::Packet> pkt = ns3::Create<ns3::Packet> (data, len);
              sendSockets.at (i)->Send (pkt);
            }

        }
    }
}

WiselibExtIface::node_id_t 
WiselibExtIface::id ()
{
  return 0;
}

void
WiselibExtIface::initRadio ()
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

  // defaut 3 nodes
  nodes.Create (3);

  // The below set of helpers will help us to put together the wifi NICs we want
  ns3::WifiHelper wifi;
  wifi.SetStandard (ns3::WIFI_PHY_STANDARD_80211b);

  ns3::YansWifiPhyHelper wifiPhy =  ns3::YansWifiPhyHelper::Default ();
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
  ns3::NqosWifiMacHelper wifiMac = ns3::NqosWifiMacHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",ns3::StringValue (phyMode),
                                "ControlMode",ns3::StringValue (phyMode));

  // Set it to adhoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac");
  ns3::NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, nodes);

  // Note that with FixedRssLossModel, the positions below are not 
  // used for received signal strength. 

  ns3::MobilityHelper mobility;
  ns3::Ptr<ns3::ListPositionAllocator> positionAlloc = ns3::CreateObject<ns3::ListPositionAllocator> ();
  positionAlloc->Add (ns3::Vector (5.0, 0.0, 0.0));
  positionAlloc->Add (ns3::Vector (0.0, 5.0, 0.0));
  positionAlloc->Add (ns3::Vector (0.0, 0.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);

  ns3::InternetStackHelper internet;
  internet.Install (nodes);

  ns3::Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  ns3::Ipv4InterfaceContainer i = ipv4.Assign (devices);

  ns3::TypeId tid = ns3::TypeId::LookupByName ("ns3::UdpSocketFactory");
  for (uint16_t i = 0; i < nodes.GetN () - 1; i++)
    {
      ns3::Ptr<ns3::Socket> recvSink = ns3::Socket::CreateSocket (nodes.Get (i), tid);
      //ns3::InetSocketAddress local = ns3::InetSocketAddress (ns3::Ipv4Address::GetAny (), 80);
      //recvSink->Bind (local);
      //the call back methods for sink nodes are setted by regRecvCallback
      //recvSink->SetRecvCallback (ns3::MakeCallback (&ReceivePacket));
      recvSockets.push_back (recvSink);
    }

  ns3::Ptr<ns3::Socket> source = ns3::Socket::CreateSocket (nodes.Get (nodes.GetN () - 1), tid);
  //ns3::InetSocketAddress remote = ns3::InetSocketAddress (ns3::Ipv4Address::GetBroadcast (), 80);
  //source->SetAllowBroadcast (true);
  //source->Connect (remote);
  sendSockets.push_back (source);

}


}

