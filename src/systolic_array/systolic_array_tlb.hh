#ifndef __SYSTOLIC_ARRAY_TLB_HH
#define __SYSTOLIC_ARRAY_TLB_HH

#include <map>
#include <set>
#include <deque>
#include <unordered_map>
#include <utility>

#include "base/statistics.hh"
#include "mem/packet.hh"
#include "mem/request.hh"

namespace gem5{

class BaseTLBMemory {

 public:
  virtual ~BaseTLBMemory() {}
  virtual void clear() = 0;
  virtual bool lookup(Addr vpn, Addr& ppn, bool set_mru = true) = 0;
  // Inserts a translation into the TLB.
  virtual void insert(Addr vpn, Addr ppn) = 0;
  // Name of the TLB structure for printing traces.
};

class InfiniteTLBMemory : public BaseTLBMemory {
  std::map<Addr, Addr> entries;

 public:
  InfiniteTLBMemory() {}
  ~InfiniteTLBMemory() {}

  virtual bool lookup(Addr vpn, Addr& ppn, bool set_mru = true) {
    auto it = entries.find(vpn);
    if (it != entries.end()) {
      ppn = it->second;
      return true;
    } else {
      ppn = Addr(0);
      return false;
    }
  }

  virtual void insert(Addr vpn, Addr ppn) { entries[vpn] = ppn; }

  virtual void clear() { entries.clear(); }
};

}
#endif
