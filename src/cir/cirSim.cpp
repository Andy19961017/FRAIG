/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include <string>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

extern RandomNumGen rnGen;
extern unsigned short SimIterator;

HashSet<PatNode>*_PatHash = new HashSet<PatNode>;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{
	unsigned piNum = I, n = AA + 64 - (AA % 64);
	for (unsigned i = 0; i < n; ++i)
		_PiPat.push_back(PiPattern(piNum));
	cout << _PiPat.size() << " patterns simulated." << endl;
	SimPattern();
}

void
CirMgr::fileSim(ifstream& patternFile)
{
	if (!patternFile.good()) { cerr << "Error: can't open !" << endl; return; }

	_PiPat.clear();
	// string str(""), str2("");
	string str("");
	char a[]="01", b[]=" \n\r";
	unsigned piNum = I;
	unsigned count = 0;
	string s(istreambuf_iterator<char>(patternFile), {});

		size_t pos = 0, pos2 = 0;
		if (s.length() == 0) return;
		while (pos != (size_t)(-1))
		{
			pos2 = s.find_first_not_of(b, pos);
			if (pos2 == (size_t)(-1)) break;
			pos = s.find_first_of(b, pos2);
// cout << pos << ' ' << pos2 << endl;
			str = s.substr(pos2,pos - pos2);
			pos2 = pos;
			if (str.length() != piNum){
				cerr << "Error: Pattern(" << str << ") length(" << str.length() 
				<< ") does not match the number of inputs(" << piNum << ") in a circuit!!"
				<< "\n0 patterns simulated." << endl;
				return;
			}	
			if (str.find_first_not_of(a) != string::npos){	
				cerr << "Error: Pattern(" << str << ") contains a non-0/1 character(‘"
				<< str[str.find_first_not_of(a)] << "’)." << "\n0 patterns simulated." << endl;
				return;
			}
			_PiPat.push_back(PiPattern(str));
			count++;
	}
	patternFile.close();
	_NumOfPat = _PiPat.size();
	cout << _NumOfPat << " patterns simulated." << endl;
	if (_PiPat.empty()) return;
	if (count % 64 != 0)
	{
		size_t k = 64 - (count % 64);
		for (unsigned i = 0; i < k; ++i)
			_PiPat.push_back(string(piNum, '0'));
	}
	SimPattern();
}

void PlusSimIter()
{
	if (SimIterator == 63)
	{
		SimIterator=0;
		return;
	}
	else SimIterator++;
}

void CirMgr::SimPattern()
{
	if (_PiPat.size()%64 != 0)
	{
		for (unsigned i = 0, n = 64 - (_PiPat.size()%64); i < n; ++i)
			_PiPat.push_back(PiPattern(I));  //for FRAIG calling
	}
	unsigned n = I;
	unsigned q = _DFSList.size();
	_totalList[0]->ConstSetPatList(_PiPat.size()/64);   //set _SimPatList of CONST

	for (unsigned i = 0, k = M+O; i < k; ++i)
		if (_totalList[i]) _totalList[i]->ClearSimPatList();
	for (unsigned i = 0, k = _PiPat.size(); i < k; i += 64)
	{
		for (unsigned t = 0; t < n; ++t)
			_PiList[t]->ResetSimPat();
		for (unsigned c = 0; c < 64; ++c)
		{
			for (unsigned j = 0; j < n ; ++j)
				_PiList[j]->SetPat(_PiPat[i+c][j]);
			PlusSimIter();
		}
		for (unsigned l = 0 ; l < q ; ++l)
			_DFSList[l]->sim();
	}
	if (_simLog) OutputToLog();
	clear_PiPat();
	ConstructFCC();
}

void CirMgr::OutputToLog()
{
	for (unsigned i = 0, k = _NumOfPat; i < k; ++i)
	{
		for (unsigned j = 0, n = I; j < n; ++j)
			*_simLog << _PiPat[i][j];
		*_simLog << ' ';
		for (unsigned j = M+1, n = M+O; j <= n ; ++j)
			 *_simLog << _totalList[j]->GetSinglePat(i);
			// cout < < j;
			// *_simLog << ((((*(_totalList[j]->getPat()))[i/64]) << (63-(i%64)) ) >> 63 );
		// }
		*_simLog << '\n';
	}
	delete _simLog;
	_simLog = 0;
}

void CirMgr::ConstructFCC()
{
// PatNode x(_totalList[6]->getPat(), 6);
// PatNode y(_totalList[8]->getPat(), 8);
// cout <<  "6 == 5 " << (x == y) << endl;
	_PatHash->clear();
	if (!_FCCList.empty()){
		for (unsigned i = 0, n = _DFSList.size(); i < n; ++i)
			_DFSList[i]->ClearFCC();
		_totalList[0]->ClearFCC();
		ClearFCCList();
		_FCCList.clear();
	}
	// for (unsigned i = 0, n = M; i < n ; ++i)
		// if (_totalList[i])_totalList[i]->setFCCID(0);
	PatNode C(_totalList[0]->getPat(), 0);			//INSERT CONST0 IN THE HASH
// cout << "const pat size " << (_totalList[0]->getPat())->size() << endl;
		_PatHash->insert(C);

	// CirGate::setGlobRef();
	// setDFStoRef();
	for (unsigned i = 1, n = _totalList.size(); i < n; ++i)
	{
		if (!_totalList[i]) continue;
		if (!_totalList[i]->isGlobRef()) continue;
		if (!_totalList[i]->isAig()) continue;
		PatNode P(_totalList[i]->getPat(), _totalList[i]->getId());
		PatNode* G = (PatNode*)_PatHash->Find(P);
// cout << "(*(_DFSList[i]->getPat()))[0] " << endl << (*(_DFSList[i]->getPat())).size() << endl;
		if (G)
			SameFCC(G->GateId(),P.GateId());
// {
// cout << " G->GateId " << G->GateId() << " P.GateId() " << P.GateId() << endl;
// cout << "*G == P " << ((*G) == P) << endl;

// }
	}
	_StructHash->clear();
}

bool myComp(const FCC* const a, const FCC* const b)
{
	return (a->_GateList.front().gate()->getId()) < (b->_GateList.front().gate()->getId());
}

void CirMgr::SameFCC(const unsigned& p, const unsigned& l)  //p is the one with smaller id
{
// if (p == 3408 && l == 3407) cout << "same fcc " << p << " " << l  << endl;
	bool inv = ( _totalList[p]->GetFirstPat() != _totalList[l]->GetFirstPat() );
	if (_totalList[p]->getFCCID() == 0)
	{
		FCC* F = new FCC((CirGateV(_totalList[p],false)));
		_FCCList.push_back(F);
		*(_FCCList.back()) += CirGateV(_totalList[l],inv);
		_totalList[p]->setFCCID(F);
		_totalList[l]->setFCCID(F);
	}
	else
	{
		_totalList[l]->setFCCID(_totalList[p]->getFCCID());
		*(_totalList[p]->getFCCID()) += CirGateV(_totalList[l],inv);
	}
	sort(_FCCList.begin(), _FCCList.end(), myComp);
}

void CirMgr::testPattern()
{
	for (unsigned i = 0; i < _PiPat.size(); ++i)
		cout << _PiPat[i];
	cout << "size: " << _PiPat.size() << endl;
}

ostream & operator<<(ostream &os, PiPattern const &m) { 
  for (int i = 0; i < 42; ++i)
  {
    os << m._pat[i];
  }
  os << endl;
    return os;
}

void CirMgr::clear_PiPat()
{
	for (unsigned i = 0, n = _PiPat.size(); i < n; ++i){
		_PiPat[i].delPat();
	}
	_PiPat.clear();
}

ostream & operator<<(ostream &os, const FCC &m) { 
  for (unsigned i = 0, n = m._GateList.size(); i < n; ++i)
  {
   	os << ' ';
  	if (m._GateList[i].isInv()) os << '!';
    os << m._GateList[i].gate()->getId();
  }
    return os << endl;
}
/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
