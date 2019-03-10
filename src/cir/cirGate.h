/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"
#include "sat.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

class CirGate;


//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------

/*class CirGateV 
{
  //if the gate is defined, the last bit is INV
  //if the gata is not defines, the second last bit is 1, and the bits in the front is gate id
    friend CirMgr;
    friend CirGate;
    friend PIGate;
    friend POGate;
    friend AIGGate;
    friend ConstGate;
  CirGateV(CirGate* g, bool inv): _gateV(g), _inv(inv)  { }
  CirGate* gate() const { return (CirGate*)(_gateV); } 
  bool isInv() const { return _inv; }
  CirGate* _gateV;
  bool _inv;
};*/


class CirGate
{
  friend class CirMgr;
public:
  CirGate(unsigned i, unsigned l): _id(i), _LineNo(l), _ref(0) {};
  virtual ~CirGate(){};

  // Basic access methods
  virtual string getTypeStr() const = 0;
  virtual GateType getGateType() const = 0;
  virtual string getName() const { return ""; }
  unsigned getLineNo() const { return _LineNo; }
  unsigned getId() const { return _id; }
  virtual void setName(string) {};
  virtual bool isAig() const { return false; }
  virtual bool isUndef() const { return false; }
  virtual bool isConst() const { return false; }
  virtual bool isPi() const { return false; }
  virtual bool isPo() const { return false; }

  // Printing functions
  virtual void printGate() const = 0;
  void reportGate() const;
  void reportFanin(int level);
  void reportFanout(int level);
  virtual void rFanin(int level, int spaceNo, bool inv);
  virtual void rFanout(int level, int spaceNo, bool inv);

  static void setGlobRef() { _globRef++; }
  void setRef() { _ref=_globRef; }
  bool isGlobRef() const { return _ref==_globRef; }
  void setDFSList() const;

  virtual void sweep() {};
  virtual void optimize() {};
  vector<CirGateV>& outList() { return _outList; }
  void merge(CirGate* g, bool inv);
  void AppendOutlist(vector<CirGateV>& list, bool& inv);
  void ChangeOutIn(const CirGate* const& g, bool& inv) const;
  void ChangeIn(const CirGate* const& f, const CirGate* const& t, bool& inv);
  void PopInOut() const;
  void PopOut(const CirGate* const& g);
  virtual void sim() {};
  virtual void ClearSimPat(){};
  virtual void SetPat(const bool&) {};
  virtual bool p() const { return false; }
  virtual void PushPat() {};
  virtual void PrintSimValue() const;
  virtual void ConstSetPatList(unsigned) {};
  virtual const vector<size_t>* getPat() { return NULL; }
  virtual void setFCCID(FCC* const & fcc) {};
  virtual FCC* getFCCID() { return 0; } //dummy
  virtual const bool GetFirstPat() const { return _ref; } //dummy
  virtual void PrintGateFCC() const {};
  virtual void ResetSimPat() {};
  virtual const size_t& GetSimPat() const { return _ref; }; //dummy
  virtual bool GetSinglePat(unsigned& i) const { return false; }
  virtual void ClearSimPatList(){};
  virtual void SetVar(const Var&) {};
  virtual Var GetVar() const { return 0; }
  virtual bool ExistInFCC() { return false; }
  virtual bool InvFCC() const { return false; }
  virtual void ClearFCC() {};
  virtual CirGate* Leader() const { return 0; }

protected:
  vector<CirGateV> _inList;
  vector<CirGateV> _outList;
  const unsigned _id;
  const unsigned _LineNo;
  size_t _ref;
  static size_t _globRef;
};



class PIGate:public CirGate
{
friend CirMgr;
public:
  PIGate(unsigned i, unsigned l): CirGate(i,l), _name(""), _SimPat(0) {};
  virtual ~PIGate(){};

  string getTypeStr() const { return "PI"; }
  GateType getGateType() const { return PI_GATE; }
  virtual string getName() const { return _name; }
  void printGate() const;
  virtual void setName(string s) { _name=s; };
  virtual void rFanin(int level, int spaceNo, bool inv);
  virtual bool isPi() const { return true; }
  // virtual void sim();
  // virtual void ClearSimPat();
  virtual void SetPat(const bool&);
  virtual bool p() const;
  // virtual void PushPat();
  virtual void PrintSimValue() const;
  virtual void ResetSimPat() { _SimPat = 0; }
  virtual const size_t& GetSimPat() const { return _SimPat; }
  virtual void SetVar(const Var& v) { _var = v; }
  virtual Var GetVar() const { return _var; }

private:
  string _name;
  size_t _SimPat;
  Var _var;
};

class POGate:public CirGate
{
friend CirMgr;
public:
  POGate(unsigned i, unsigned l): CirGate(i,l), _name(""), _SimPat(0) {};
  ~POGate(){};

  string getTypeStr() const { return "PO"; }
  GateType getGateType() const { return PO_GATE; }
  virtual string getName() const { return _name; }
  void printGate() const;
  virtual void setName(string s) { _name=s; };
  virtual void rFanout(int level, int spaceNo, bool inv);
  virtual bool isPo() const { return true; }
  virtual void optimize();
  virtual void sim();
  // virtual void ClearSimPat();
  // virtual void SetPat(const bool&);
  virtual bool p() const;
  // virtual void PushPat();
  virtual void PrintSimValue() const;
  virtual const vector<size_t>* getPat() { return &_SimPatList; }
  virtual bool GetSinglePat(unsigned& i) const;
  virtual void ClearSimPatList() { _SimPatList.clear(); }
  // virtual void ResetSimPat() { _SimPat = 0; }

private:
  string _name;
  vector<size_t> _SimPatList;
  size_t _SimPat;
};

class AIGGate:public CirGate
{
public:
  AIGGate(unsigned i, unsigned l): CirGate(i,l), _SimPat(0), _FCC(0) {};
  ~AIGGate(){};
  
  string getTypeStr() const { return "AIG"; }
  GateType getGateType() const { return AIG_GATE; }
  void printGate() const;
  virtual bool isAig() const { return true; }
  virtual void optimize();

  virtual void sweep();
  void OutMerge(const CirGateV) const;
  void InMerge() const;
  virtual void sim();
  virtual void ClearSimPat();
  // virtual void SetPat(const bool&);
  virtual bool p() const;
  virtual void PushPat();
  virtual void PrintSimValue() const;
  virtual const vector<size_t>* getPat() { return &_SimPatList; }
  virtual void setFCCID(FCC* const & fcc) { _FCC = fcc; }
  virtual FCC* getFCCID() { return _FCC; }
  virtual const bool GetFirstPat() const { return _SimPat%2; } 
  virtual void PrintGateFCC() const;
  virtual const size_t& GetSimPat() const { return _SimPat; }
  virtual void ClearSimPatList() { _SimPatList.clear(); }
  virtual void SetVar(const Var& v) { _var = v; }
  virtual Var GetVar() const { return _var; }
  virtual bool ExistInFCC() const;
  virtual bool InvFCC() const;
  virtual void ClearFCC() { _FCC = 0; }
  virtual CirGate* Leader() const;


private:
  vector<size_t> _SimPatList;
  size_t _SimPat;
  FCC* _FCC;
  Var _var;
};

class ConstGate:public CirGate
{
public:
  ConstGate(): CirGate(0,0), _FCC(0) {};
  ~ConstGate(){};

  string getTypeStr() const { return "CONST"; }
  GateType getGateType() const { return CONST_GATE; }
  void printGate() const { cout << "CONST0"; }
  virtual bool isConst() const { return true; }
  virtual void ConstSetPatList(unsigned);
  virtual const vector<size_t>* getPat() { return &_SimPatList; }
  virtual void setFCCID(FCC* const & fcc)  { _FCC = fcc; }
  virtual FCC* getFCCID() { return _FCC; }
  virtual const bool GetFirstPat() const { return false; } 
  virtual void PrintGateFCC() const;
  virtual void SetVar(const Var& v) { _var = v; }
  virtual Var GetVar() const { return _var; }
  virtual void ClearFCC() { _FCC = 0; }

private:
  vector<size_t> _SimPatList;
  FCC* _FCC;
  Var _var;
};

class UNDEFGate:public CirGate
{
public:
  UNDEFGate(unsigned i): CirGate(i,0) {};
  ~UNDEFGate(){};
  
  string getTypeStr() const { return "UNDEF"; }
  GateType getGateType() const { return UNDEF_GATE; }
  void printGate() const { cout << "UNDEF " << _id << endl; }
  virtual bool isUndef() const { return true; }
  virtual void sweep();
};

class FCC
{
  friend CirMgr;
  friend AIGGate;
  friend ostream & operator<<(ostream &os, const FCC &m);
  friend bool myComp(const FCC* const a, const FCC* const b);
public:
  FCC(const CirGateV& gate) { _GateList.push_back(gate); }
  ~FCC(){};

  void operator+=(const CirGateV& gate) { _GateList.push_back(gate); }
  void PrintGateExp(const unsigned& id)
  {
    bool inv = false;
    for (unsigned i = 0, n = _GateList.size(); i < n; ++i)
      if (_GateList[i].gate()->getId()==id){
        inv = _GateList[i].isInv();
        break;
      }
    for (unsigned i = 0, n = _GateList.size(); i < n; ++i)
    {
      if (_GateList[i].gate()->getId()==id) continue;
      if (_GateList[i].isInv() != inv) cout << '!';
      cout << _GateList[i].gate()->getId();
      if (i != n-1)
       cout << ' ';
    }
  }

private:
  vector<CirGateV> _GateList;  // pointer with inv info
};



#endif // CIR_GATE_H
