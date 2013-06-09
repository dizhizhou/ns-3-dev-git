/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

// Author: Dizhi Zhou (dizhi.zhou@gmail.com)

#ifndef WISELIB_EXT_IFACE_H
#define WISELIB_EXT_IFACE_H

namespace wiselib {

class WiselibExtIface
{
public:
  WiselibExtIface ();

  virtual ~WiselibExtIface ();

  void Debug (const char *msg);

};

}

#endif /* WISELIB_EXT_IFACE_H */

