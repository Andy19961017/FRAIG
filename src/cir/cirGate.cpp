/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include<cmath>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr *cirMgr;

size_t CirGate::_globRef=0;
unsigned short SimIterator;


/**************************************/
/*   class CirGate member functions   */
/**************************************/

void CirGate::reportGate() const
{
	cout << "================================================================================\n= "
	<< this->getTypeStr();
	cout << '(' << this->_id << ')';
	if (this->getName() != "")
		cout << '\"' << this->getName() << '\"';
	cout << ", line " << this->_LineNo;
	cout << "\n= FECs: ";
	PrintGateFCC();
	cout << "\n= Value: ";
	PrintSimValue();
	cout << "\n================================================================================" << endl;
}

void CirGate::reportFanin(int level) { setGlobRef(); this->rFanin(level,0,0); }
void CirGate::reportFanout(int level) { setGlobRef(); this->rFanout(level,0,0); }

void CirGate::rFanin(int level, int spaceNo, bool inv)
{
	for (int i = 0; i < spaceNo; ++i) cout << "  ";
	if (inv) cout << '!';
	cout << this->getTypeStr() << ' ' << this->_id;
	if (this->getGateType()==PI_GATE || this->getGateType()==CONST_GATE 
		|| this->getGateType()==UNDEF_GATE)  { cout << endl; return; }
	if (this->isGlobRef())
		cout << " (*)" << endl;
	else
	{
		this->setRef();
		cout << endl;
		if (level==0) return;
		level--;
		spaceNo++;
		this->_inList[0].gate()->rFanin(level,spaceNo,this->_inList[0].isInv());
		if (this->isAig())
			this->_inList[1].gate()->rFanin(level,spaceNo,this->_inList[1].isInv());
	}
}

void PIGate::rFanin(int level, int spaceNo, bool inv)
{
	for (int i = 0; i < spaceNo; ++i) cout << "  ";
	if (inv) cout << '!';
	cout << this->getTypeStr() << ' ' << this->_id << endl;
}


void CirGate::rFanout(int level, int spaceNo, bool inv)
{
	for (int i = 0; i < spaceNo; ++i) cout << "  ";
	if (inv) cout << '!';
	cout << this->getTypeStr() << ' ' << this->_id;
	if (this->getGateType()==PO_GATE) { cout << endl; return; }
	if (level==0) { cout << endl; return; }
	if (this->isGlobRef())
		cout << " (*)" << endl;
	else
	{
		cout << endl;
		level--;
		spaceNo++;
		for (size_t i = 0, n = this->_outList.size(); i < n; ++i)
		{
			this->setRef();
			this->_outList[i].gate()->rFanout(level,spaceNo,this->_outList[i].isInv());
		}
	}
}

void POGate::rFanout(int level, int spaceNo, bool inv)
{
	for (int i = 0; i < spaceNo; ++i) cout << "  ";
	if (inv) cout << '!';
	cout << this->getTypeStr() << ' ' << this->_id << endl;
}

void CirGate::setDFSList() const
{
	for (size_t i = 0; i < _inList.size() ; ++i)
	{
		if (this->_inList[i].gate()->getGateType() != UNDEF_GATE)
		{
			if (!this->_inList[i].gate()->isGlobRef())
			{
				this->_inList[i].gate()->setRef();
				this->_inList[i].gate()->setDFSList();
			}
		}
	}
	cirMgr->_DFSList.push_back((CirGate*)this);
}

void PIGate::printGate() const 
{
	cout << "PI  " << _id;
	if (_name!="") cout << " (" << _name << ')';
}

void POGate::printGate() const 
{
	cout << "PO  " << _id << ' ';
	if (this->_inList[0].gate()->getGateType() == UNDEF_GATE) cout << '*';
	if (_inList[0].isInv()) cout << '!';
	cout << _inList[0].gate()->getId();
	if (_name!="") cout << " (" << _name << ')';
}

void AIGGate::printGate() const 
{
	cout << "AIG " << _id << ' ';
	if (this->_inList[0].gate()->getGateType() == UNDEF_GATE) cout << '*';
	if (_inList[0].isInv()) cout << '!';
	cout << _inList[0].gate()->getId();
	cout << ' ';
	if (this->_inList[1].gate()->getGateType() == UNDEF_GATE) cout << '*';
	if (_inList[1].isInv()) cout << '!';
	cout << _inList[1].gate()->getId();
}

void UNDEFGate::sweep()
{
	cout << "Sweeping: " << getTypeStr() << '(' << _id << ") removed...\n";
	cirMgr->delGate(_id);
}


void AIGGate::sweep()
{
	cout << "Sweeping: " << getTypeStr() << '(' << _id << ") removed...\n";
	cirMgr->delGate(_id);
	CirGate* V=_inList[0].gate();
	if (V->isGlobRef() || V->isPi())
		for (unsigned i = 0, n=V->outList().size(); i < n; ++i)
		{
			if (V->outList()[i].gate()==this)
			{
				V->outList()[i]=V->outList().back();
				V->outList().pop_back();
				break;
			}
		}
	CirGate* V2=_inList[1].gate();
	if (V2->isGlobRef()|| V->isPi())
		for (unsigned i = 0, n=V2->outList().size(); i < n; ++i)
		{
			if (V2->outList()[i].gate()==this)
			{
				V2->outList()[i]=V2->outList().back();
				V2->outList().pop_back();
				break;
			}
		}
}

void POGate::optimize()
{
	_inList[0].gate()->optimize();
}

void pirntSimp()
{
	cout << "Simplifying: ";
}

void AIGGate::optimize()
{
	this->setRef();
	if (!_inList[0].gate()->isGlobRef()) _inList[0].gate()->optimize();
	if (!_inList[1].gate()->isGlobRef()) _inList[1].gate()->optimize();

	if (_inList[0].gate()->isConst()){     	//IS CONST
		if (_inList[0].isInv())
			{ pirntSimp(); merge(_inList[1].gate(),_inList[1].isInv()); }	// CHANGE TO THE OTHER
		else { pirntSimp(); merge(_inList[0].gate(),false); }  //CHANGE TO CONST0
	}    
	else if (_inList[1].gate()->isConst()){    //IS CONS
		if (_inList[1].isInv())
			{ pirntSimp(); merge(_inList[0].gate(),_inList[0].isInv()); }	// CHANGE TO THE OTHER
		else { pirntSimp(); merge(_inList[1].gate(),false); }
	}	
	else if (_inList[0].sameGate(_inList[1])){    //same gate
		if (_inList[0].isInv() == _inList[1].isInv())   // same phase
			{ pirntSimp(); merge(_inList[0].gate(),_inList[0].isInv()); }  // same as input
		else { pirntSimp(); merge(cirMgr->_totalList[0],false); }   //CONST0
	}
	else return;

	cirMgr->delGate(_id);
}

void CirGate::merge(CirGate* g, bool inv)
{
	cout << g->getId() << " merging ";
	if (inv) cout << '!';
	cout << _id << "..." << endl;
	g->AppendOutlist(_outList,inv);
	ChangeOutIn(g,inv);
	PopInOut();
}

void CirGate::AppendOutlist(vector<CirGateV>& list, bool& inv)
{
	if (inv)
		for (unsigned i = 0, n = list.size(); i < n; ++i)
			list[i]._gateV ^= (size_t)inv;
	_outList.insert(end(_outList), begin(list), end(list));
}

void CirGate::ChangeOutIn(const CirGate* const& g, bool& inv) const
{
	for (unsigned i = 0, n = _outList.size(); i < n; ++i)
		_outList[i].gate()->ChangeIn(this,g,inv);
}

void CirGate::ChangeIn(const CirGate* const& f, const CirGate* const& t, bool& inv) 
{
	if (_inList[0].gate()==f)
		_inList[0]._gateV = (size_t) t | (size_t)(inv ^ _inList[0].isInv());
	if (isPo()) return;
	if (_inList[1].gate()==f)
		_inList[1]._gateV = (size_t) t | (size_t)(inv ^ _inList[1].isInv());
}

void CirGate::PopInOut() const
{
	_inList[0].gate()->PopOut(this);
	_inList[1].gate()->PopOut(this);
}

void CirGate::PopOut(const CirGate* const& g)
{
	for (unsigned i = 0, n = _outList.size(); i < n ; ++i){
		if (_outList[i].gate()==g){
			_outList[i]=_outList.back();
			_outList.pop_back();
			return;
		}
	}
}

// void PIGate::sim()
// {
	// setRef();
// }

void POGate::sim()
{
	_SimPat = 
	((_inList[0].isInv()) ? ~(_inList[0].gate()->GetSimPat()) : (_inList[0].gate()->GetSimPat()));	
	_SimPatList.push_back(_SimPat);
}

void AIGGate::sim()
{
	_SimPat = 
	((_inList[0].isInv()) ? ~(_inList[0].gate()->GetSimPat()) : (_inList[0].gate()->GetSimPat()))
	&
	((_inList[1].isInv()) ? ~(_inList[1].gate()->GetSimPat()) : (_inList[1].gate()->GetSimPat()));
	_SimPatList.push_back(_SimPat);
}

// void PIGate::ClearSimPat()
// {
	// setRef();
	// _SimPatList.clear(); _SimPat = 0;	
// }

// void POGate::ClearSimPat()
// {
	// _SimPatList.clear(); _SimPat = 0;
	// if (!_inList[0].gate()->isGlobRef())
		// _inList[0].gate()->ClearSimPat();
// }

void AIGGate::ClearSimPat()
{
	// setRef();
	_SimPatList.clear();
	// if (!_inList[0].gate()->isGlobRef())
		// _inList[0].gate()->ClearSimPat();
	// if (!_inList[1].gate()->isGlobRef())
		// _inList[1].gate()->ClearSimPat();
}

void PIGate::SetPat(const bool& p)
{
// cout << "gate " << _id << " set " << p  << " SimIterator " << SimIterator << endl;
	_SimPat |= ((size_t) p << SimIterator);
// if (_id == 1) cout << "SetPat " << _id << ' ' << p << endl;
}

// void POGate::SetPat(const bool& p)
// {
	// _SimPat |= ((size_t) p << SimIterator);
// }

// void AIGGate::SetPat(const bool& p)
// {
	// _SimPat |= ((size_t) p << SimIterator);
// }

// void PIGate::PushPat()
// {
// cout << "PushPat " << _id << endl;
	// setRef();
	// _SimPatList.push_back(_SimPat);
// }

// void POGate::PushPat()
// {
// cout << "PushPat " << _id << endl;
	// if (!_inList[0].gate()->isGlobRef())
		// _inList[0].gate()->PushPat();
		// _SimPatList.push_back(_SimPat);
// }

void AIGGate::PushPat()
{
// cout << "PushPat " << _id << endl;
	// setRef();
	_SimPatList.push_back(_SimPat);
	// if (!_inList[0].gate()->isGlobRef())
		// _inList[0].gate()->PushPat();
	// if (!_inList[1].gate()->isGlobRef())
		// _inList[1].gate()->PushPat();
}

bool PIGate::p() const { return ( _SimPat << (63 - SimIterator) ) >> 63; }

bool POGate::p() const { return ( _SimPat << (63 - SimIterator) ) >> 63; }

bool AIGGate::p() const { return ( _SimPat << (63 - SimIterator) ) >> 63; }

void CirGate::PrintSimValue() const
{
	cout << "00000000_00000000_00000000_00000000_00000000_00000000_00000000_00000000";
}

void PIGate::PrintSimValue() const
{
// cout << "_SimPat " << _SimPat << endl;
	for (int i = 63; i >= 0; --i)
	{
		cout << (bool)(_SimPat & ( (size_t)(1) << i ) );
		if (i % 8 == 0)
			if (i != 0)
				cout << '_';
	}
}

void POGate::PrintSimValue() const
{
	for (int i = 63; i >= 0; --i)
	{
		cout << (bool)(_SimPat & ( (size_t)(1) << i ) );
		if (i % 8 == 0)
			if (i != 0)
				cout << '_';
	}
}

void AIGGate::PrintSimValue() const
{
	for (int i = 63; i >= 0; --i)
	{
		cout << (bool)(_SimPat & ( (size_t)(1) << i ) );
		if (i % 8 == 0)
			if (i != 0)
				cout << '_';
	}
}

void ConstGate::ConstSetPatList(unsigned k)
{
	unsigned s = _SimPatList.size();
	if ( s < k ){
		for (; s < k; ++s)
			_SimPatList.push_back(0);
	}
	else if (s > k)
		for (; s > k; --s)
			_SimPatList.pop_back();
}

void AIGGate::PrintGateFCC() const
{
	if (!_FCC) return;
// cout << "_FCCID " << _FCCID << endl;
// cout << "cirMgr->_FCCList.size() " << cirMgr->_FCCList.size() << endl;
	// cout << cirMgr->_FCCList[_FCCID];

	_FCC->PrintGateExp(_id);
}

void ConstGate::PrintGateFCC() const
{
	if (!_FCC) return;
	_FCC->PrintGateExp(_id);
}

bool POGate::GetSinglePat(unsigned& i) const
{
	if (i/64 >= _SimPatList.size() ) return 0;
	return (((_SimPatList[i/64]) << (63-(i%64)) ) >> 63 );
}

bool AIGGate::InvFCC() const 
{ return this->GetFirstPat() != _FCC->_GateList.front().gate()->GetFirstPat(); }

bool AIGGate::ExistInFCC() const
{
cout << "ExistInFCC() " << _id << " this " << this << " _FCC->_GateList.front().gate() "
	 << _FCC->_GateList.front().gate() << endl;
 return this != _FCC->_GateList.front().gate(); 
}

CirGate* AIGGate::Leader() const
{
	return _FCC->_GateList.front().gate();
}

