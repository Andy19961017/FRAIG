/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;
// extern HashSet<StructNode>* _StructHash;
// extern HashSet<PatNode>* _PatHash;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
// static unsigned colNo  = 0;  // in printing, colNo needs to ++
// static char buf[1024];
// static string errMsg;
// static int errInt;
// static CirGate *errGate;

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/

void myGetline(istream& file, string& option) { getline(file, option); lineNo++; }

CirMgr::~CirMgr()
{
  for (size_t i = 0, n=_totalListBackUp.size(); i < n ; ++i)
    if (_totalListBackUp[i]) delete _totalListBackUp[i];
  // if (_solver) delete _solver;
  ClearFCCList();
}

bool
CirMgr::readCircuit(const string& fileName)
{
  lineNo=0;
  string option;
  vector<string> options;
  ifstream file(fileName.c_str());
  if (!file.good()) return false;
  myGetline(file, option);
  lexOptions(option,options,6);

  for (int i = 1, n=options.size(); i < n; ++i)
  {
    int temp;
    if (!myStr2Int(options[i], temp)) cerr << "ERROR" << endl; 
    _header.push_back(temp);
  }

  for (int i = 0, n=M+O; i <= n ; ++i)
    _totalList.push_back(0);

  _totalList[0]=new ConstGate();
  for (int i = 0, n=I; i < n; ++i)
  {
    int id;
    string idStr;
    myGetline(file,idStr);
    if (!myStr2Int(idStr, id)) cerr << "ERROR" << endl;
    _PIorder.push_back(id);
    _totalList[id/2]=new PIGate(id/2,lineNo);
    _PiList.push_back(_totalList[id/2]);
  }

  for (int i = M+1, n=i+O; i < n; ++i)
  {
    myGetline(file,option);
    _totalList[i]=new POGate(i,lineNo);
  }

  for (int i = 0, n=AA; i < n; ++i)
  {
    myGetline(file,option);
    size_t pos=0;
    pos=option.find_first_of(' ');
    if (pos != string::npos) option=option.substr(0,pos);
    int id=0;
    if (!myStr2Int(option, id)) { cerr << "error" << " option= " << option << endl;  }
    _totalList[(id/2)]=new AIGGate(id/2,lineNo);
  }

  file.close();
  file.open(fileName.c_str());

  for (int i = 0, n=1+I; i < n; ++i) getline(file,option);

  for (int i = M+1, n=M+O+1; i < n; ++i)
  {
    getline(file,option);
    int id=0;
    if (!myStr2Int(option, id)) { cerr << "error" << " option= " << option << endl;  }
    _POorder.push_back(id);
    connect(id/2,i,id%2);
  }
  
  for (unsigned i = 0; i < AA; ++i)
  {
    getline(file,option);
    options.clear();
    lexOptions(option,options);
    int thisId=0, Id1=0, Id2=0;
    if (!myStr2Int(options[0], thisId)) cerr << "error" << endl;
    if (!myStr2Int(options[1], Id1)) cerr << "error" << endl;
    if (!myStr2Int(options[2], Id2)) cerr << "error" << endl;
    connect(Id1/2,thisId/2,Id1%2);
    connect(Id2/2,thisId/2,Id2%2);
  }

  while(file.good())
  {
    getline(file,option);
    _Name.push_back(option);

    if (option.front()=='i')
    {
      size_t pos = option.find_first_of(' ',0);
      string str = option.substr(0,pos);
      string str2 = option.substr(pos+1);
      int iNum=0;
      str.erase(0,1);
      if (!myStr2Int(str, iNum)) cerr << "error" << endl;
      setIName(iNum,str2);
    }
    else if (option.front()=='o')
    {
      size_t pos = option.find_first_of(' ',0);
      string str = option.substr(0,pos);
      string str2 = option.substr(pos+1);
      int oNum=0;
      str.erase(0,1);
      if (!myStr2Int(str, oNum)) cerr << "error" << endl;
      setOName(oNum,str2);
    }
    else
    {
      _Name.pop_back();
      break;
    }
  }

  file.close();
  connectGate();
  setDFSList();

  // _PiList.clear();         //SET PI LIST FOR SIM
  // for (unsigned i = 1, n = M; i <= n; ++i)
    // if (_totalList[i])
      // if (_totalList[i]->isPi()) _PiList.push_back(_totalList[i]);

  if (getHashSize(M+O)>_StructHash->NumOfBuck())
  {
    delete _StructHash;
    _StructHash=new HashSet<StructNode>(getHashSize(M+O));
  }

  if (getHashSize(M+O)>_PatHash->NumOfBuck())
  {
    delete _PatHash;
    _PatHash=new HashSet<PatNode>(getHashSize(M+O));
  }

  return true;
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
  unsigned count = 0;
  for (unsigned i = 1; i <= M; ++i){
    if (_totalList[i])
      if (_totalList[i]->isAig())
        count++;
  }
  cout << "\nCircuit Statistics\n" << "==================\n"
  << "  PI" << setw(12) << I << endl
  << "  PO" << setw(12) << O << endl
  << "  AIG" << setw(11) << count << endl
  << "------------------\n"
  << "  Total" << setw(9) << I+O+count << endl;
}

void
CirMgr::printNetlist() const
{
  cout << endl;
  for (int i = 0, n=_DFSList.size(); i < n; ++i)
  {
    cout << '[' << i << "] "; 
    _DFSList[i]->printGate();
    cout << endl;
  }
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for (size_t i=0, n=_PIorder.size(); i < n; i++)
   {
  cout << ' ' << _PIorder[i]/2;
   }
   cout  << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for (int i = M+1, n=i+O; i < n; ++i) 
   {
    cout << ' ' << i;
   }
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
  bool notPrinted=true;
  for (size_t i = 1, n=M+O; i <= n; ++i)
  {
    if (_totalList[i])
      if (_totalList[i]->getGateType() != PI_GATE && _totalList[i]->getGateType() != UNDEF_GATE)
        for (size_t j = 0; j < _totalList[i]->_inList.size(); ++j)
          if (_totalList[i]->_inList[j].gate()->getGateType()==UNDEF_GATE)
          {
            if (notPrinted) { cout << "Gates with floating fanin(s):"; notPrinted=false; }
            cout << ' ' << i; break; 
          }
  }
  if (!notPrinted) cout << endl;

  notPrinted=true;
  for (size_t i = 1, n=M; i <= n; ++i)
  {
    if (_totalList[i])
      if (_totalList[i]->_outList.empty() && _totalList[i]->getGateType()!=UNDEF_GATE)
      {
        if (notPrinted) { cout << "Gates defined but not used  :"; notPrinted=false; }
          cout << ' ' << i;
      }
  }
  if (!notPrinted) cout << endl;
}

void
CirMgr::printFECPairs() const
{
  for (unsigned i = 0, n = _FCCList.size(); i < n ; i++)
    cout << '[' << i << ']' << *(_FCCList[i]);
}

void
CirMgr::writeAag(ostream& outfile) const
{
  int num=0;
  for (size_t i = 0, n=_DFSList.size(); i < n; ++i)
    if (_DFSList[i]->getGateType()==AIG_GATE) num++;

  outfile << "aag " << _header[0] << ' ' << _header[1] << ' '
  << _header[2] << ' ' << _header[3] << ' ' << num;
  for (size_t i = 0, n=I; i < n; ++i)
    outfile << '\n' << _PIorder[i];
  for (size_t i = 0, n=O; i < n ; ++i)
    outfile << '\n' << _POorder[i];
  for (size_t i = 0, n=_DFSList.size(); i < n; ++i)
  {
    if (!_DFSList[i]->isAig()) continue;
    outfile << '\n' << (_DFSList[i]->_id)*2 << ' ' 
    << (_DFSList[i]->_inList[0].gate()->_id * 2 + _DFSList[i]->_inList[0].isInv()) 
    << ' ' << (_DFSList[i]->_inList[1].gate()->_id * 2 + _DFSList[i]->_inList[1].isInv());
  }
  for (int i = 0, n=_Name.size(); i < n ; ++i)
    outfile << '\n' << _Name[i];
  cout << "\nc\nAAG output by Chung-Yang (Ric) Huang\n";
}

void
CirMgr::writeGate(ostream& outfile, CirGate *g) const
{
}

bool
CirMgr::lexOptions
(const string& option, vector<string>& tokens, size_t nOpts) const
{
   string token;
   size_t n = myStrGetTok(option, token);
   while (token.size()) {
      tokens.push_back(token);
      n = myStrGetTok(option, token, n);
   }
   if (nOpts != 0) {
      if (tokens.size() < nOpts) {
cerr << "option Missing" << endl;
         return false;
      }
      if (tokens.size() > nOpts) {
cerr << "extra option" << endl;
         return false;
      }
   }
   return true;
}

void CirMgr::connect(const unsigned& pre, const unsigned& suc ,const bool& inv )
{
  if (_totalList[pre])
  {
    CirGateV temp(_totalList[pre], inv);
    _totalList[suc]->_inList.push_back(temp);
  }
  else
  {
    _totalList[pre]=new UNDEFGate(pre);
    CirGateV temp(_totalList[pre], inv);
    _totalList[suc]->_inList.push_back(temp);
  }
}

void CirMgr::setIName(const int& iNum, const string& name) const
{
  _totalList[_PIorder[iNum]/2]->setName(name);
}

void CirMgr::setOName(const int& oNum, const string& name) const
{
  _totalList[M+1+oNum]->setName(name);
}

void CirMgr::connectGate() const
{
// cout << "entered CirMgr::connectGate() " << endl;
  for (size_t i = 1, n=_totalList.size(); i < n ; ++i)
  {
    if (_totalList[i])
    {
      GateType g=_totalList[i]->getGateType();
      switch (g)
      {
        case PI_GATE:
        case CONST_GATE:
        case UNDEF_GATE:
          break;
        case PO_GATE:
        {
            CirGateV temp(_totalList[i],_totalList[i]->_inList[0].isInv());
            _totalList[i]->_inList[0].gate()->_outList.push_back(temp);
        break;
        }
        case AIG_GATE:
        {
            CirGateV temp2(_totalList[i],_totalList[i]->_inList[0].isInv());
            _totalList[i]->_inList[0].gate()->_outList.push_back(temp2);
            CirGateV temp3(_totalList[i],_totalList[i]->_inList[1].isInv());
            _totalList[i]->_inList[1].gate()->_outList.push_back(temp3);
        break;
        }
      }
    }
  }
}

void CirMgr::setDFSList() const
{
  _DFSList.clear();
  CirGate::setGlobRef();
  for (int i = M+1, n=i+O; i < n; ++i) _totalList[i]->setDFSList();
}
