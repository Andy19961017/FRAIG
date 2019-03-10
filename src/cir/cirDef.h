/****************************************************************************
  FileName     [ cirDef.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic data or var for cir package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_DEF_H
#define CIR_DEF_H

#include <vector>
#include "myHashMap.h"
#include "util.h"

using namespace std;

class CirGate;
class CirMgr;
class PIGate;
class POGate;
class AIGGate;
class ConstGate;
class SatSolver;
class FCC;

// TODO: define your own typedef or enum
typedef vector<CirGate*>           GateList;
typedef vector<unsigned>           IdList;


enum GateType
{
   UNDEF_GATE = 0,
   PI_GATE    = 1,
   PO_GATE    = 2,
   AIG_GATE   = 3,
   CONST_GATE = 4,
};

class CirGateV 
{
  #define NEG 0x1    
    friend CirMgr;
    friend CirGate;
    friend PIGate;
    friend POGate;
    friend AIGGate;
    friend ConstGate;
public:
  CirGateV(const CirGate* g,const bool inv): _gateV(size_t(g) + size_t(inv)) { }
  CirGate* gate() const { return (CirGate*)(_gateV & ~size_t(0x3)); } 
  bool isInv() const { return (_gateV & 0x1); }
  bool sameGate (const CirGateV& g) const { return (_gateV >> 1) == (g._gateV >> 1); }
private:
  size_t _gateV; 
};

class PiPattern
{
  friend CirMgr;
  friend ostream & operator<<(ostream &os, PiPattern const &m);
public:
  PiPattern(const string& pat)
  {
    _pat = new bool[pat.length()];
    for (unsigned i = 0, n = pat.length() ; i < n; ++i)
    {
      if (pat[i]=='1') _pat[i]=true;
      else _pat[i]=false;
    }
  }
  PiPattern(const unsigned& num)
  {
    _pat = new bool[num];
    for (unsigned i = 0; i < num; ++i)
      _pat[i]=rnGen(2);
  }
  PiPattern(bool*& pat):_pat(pat) {};

  bool& operator[]( unsigned& i ) const { return _pat[i]; }

  ~PiPattern() { }
  void delPat() { delete [] _pat; }
  
  bool* _pat;
};


#endif // CIR_DEF_H
