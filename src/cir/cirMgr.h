/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include "myHashSet.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"

#define M _header[0]
#define I _header[1]
#define L _header[2]
#define O _header[3]
#define AA _header[4]

extern CirMgr *cirMgr;

// class HashNode;
// class HashSet<HashNode>;

class FCC;
// HashSet<StructNode>* _StructHash;
// HashSet<PatNode>* _PatHash;

class CirMgr
{
    friend CirGate;
    friend AIGGate;
    friend ConstGate;
public:
   CirMgr():_StructHash(new HashSet<StructNode>()), 
        _PatHash(new HashSet<PatNode>()), _solver(0), _diff(0), _fraigTime(0) {}
   ~CirMgr();

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const
   { if(gid >= _totalList.size()) return 0; return _totalList[gid]; }

   // Member functions about circuit construction
   bool readCircuit(const string&);

   // Member functions about circuit optimization
   void sweep();
   void delGate(const unsigned&);
   void optimize();

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }

   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void writeAag(ostream&) const;
   void writeGate(ostream&, CirGate*) const;

private:
   void connect(const unsigned&, const unsigned&, const bool&);
   void initList();
   bool lexOptions(const string&, vector<string>&, size_t nOpts = 0) const;
   void setIName(const int&, const string&) const;
   void setOName(const int&, const string&) const;
   void setDFSList() const;
   void connectGate() const;
   void setDFStoRef() const;
   void mergeGate(const unsigned,const unsigned);
   void SimPattern();
   void testPattern();
   void clear_PiPat();
   void ConstructFCC();
   void SameFCC(const unsigned&, const unsigned&);
   void OutputToLog();
   void InitSolver();
   void DoSAT(CirGate*, CirGate*);
   void PushCounterEx();
   void FraigFCC(FCC*&);
   void ClearFCCList();


   GateList _totalList;
   GateList _totalListBackUp;
   GateList _PiList;
   mutable vector<CirGate*> _DFSList;
   vector<unsigned> _header;
   vector<unsigned> _PIorder;
   vector<unsigned> _POorder;
   vector<string> _Name;
   vector<PiPattern> _PiPat;

   HashSet<StructNode>* _StructHash;
   HashSet<PatNode>* _PatHash;

   vector<FCC*> _FCCList;
   unsigned _NumOfPat;

   SatSolver* _solver;

   ofstream           *_simLog;

   unsigned _diff;
   unsigned _fraigTime;
};


#endif // CIR_MGR_H
