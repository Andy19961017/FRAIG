/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashSet.h"
#include "util.h"

using namespace std;
HashSet<StructNode>* _StructHash = new HashSet<StructNode>;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed


void
CirMgr::strash()
{
	for (unsigned i = 0, n=_DFSList.size(); i < n; ++i)
	{
		if (!_DFSList[i]->isAig()) continue;
		StructNode N(_DFSList[i]->getId(),
			_DFSList[i]->_inList[0].gate()->getId()*2+_DFSList[i]->_inList[0].isInv(),
			_DFSList[i]->_inList[1].gate()->getId()*2+_DFSList[i]->_inList[1].isInv());
		StructNode* G = _StructHash->Find(N);
		if (G)
			mergeGate(G->GateId(),N.GateId());
	}
  	setDFSList();
	_StructHash->clear();
}

void CirMgr::mergeGate(const unsigned a,const unsigned b)
{
	cout << "Strashing: ";
	_totalList[b]->merge(_totalList[a],false);
	_totalList[b]=0;
}

void
CirMgr::fraig()
{
	_fraigTime++;
	InitSolver();
	// unsigned D = _DFSList.size();
	
	for (unsigned i = 0, n = _FCCList.size(); i < n; ++i)
		FraigFCC(_FCCList[i]);


	setDFSList();
	strash();
	if (_fraigTime < 2) SimPattern();
	delete _solver;
	if (!_FCCList.empty() && _fraigTime < 2) {
			cout << "=========== Fraig again =========== " << endl;
		fraig();
	}

	if (!_FCCList.empty()){
		for (unsigned i = 0, n = _DFSList.size(); i < n; ++i)
			_DFSList[i]->ClearFCC();
		_totalList[0]->ClearFCC();
		ClearFCCList();
		_FCCList.clear();
	}

    optimize();
	_fraigTime = 0;
	_diff = 0;
}

void CirMgr::ClearFCCList()
{
	for (unsigned i = 0; i < _FCCList.size(); ++i)
		delete _FCCList[i];
}

void CirMgr::FraigFCC(FCC*& fcc)
{
	CirGate* leader = fcc->_GateList.front().gate();
	for (unsigned i = 1, n = fcc->_GateList.size(); i < n; ++i)
		DoSAT(leader, fcc->_GateList[i].gate());
}


void CirMgr::InitSolver()
{
	_solver = new SatSolver;
	_solver->initialize();

	unsigned D = _DFSList.size();
	CirGate*& Con = _totalList[0];
	Var a = _solver->newVar();
	Con->SetVar(_solver->newVar()); //CONST

	for (unsigned i = 0, n = _PiList.size(); i < n; ++i)
		_PiList[i]->SetVar(_solver->newVar());
	for (unsigned i = 0; i < D; ++i)
		if (_DFSList[i]->isAig()) _DFSList[i]->SetVar(_solver->newVar());

	//create a const 0
	_solver->addAigCNF(Con->GetVar(), a, false, a, true); 

	//for AIG
	for (unsigned i = 0; i < D; ++i){
		CirGate*& g = _DFSList[i];
		if (g->isAig())
			_solver->addAigCNF(g->GetVar(), g->_inList[0].gate()->GetVar(), g->_inList[0].isInv(),
				g->_inList[1].gate()->GetVar(), g->_inList[1].isInv() );
	}
}

void CirMgr::DoSAT(CirGate* leader, CirGate* g)
{
	Var f = _solver->newVar();
	_solver->addXorCNF(f, g->GetVar(), g->InvFCC(), leader->GetVar(), false);
	_solver->assumeRelease();
    _solver->assumeProperty(f, true);
    if(_solver->assumpSolve())
    	PushCounterEx();
    else{
    	cout << "Fraig: ";
    	g->merge(leader,g->InvFCC());
    	_totalList[g->getId()]=0;
    }
}

void CirMgr::PushCounterEx()
{
	bool* CounterPat = new bool[I];
	for (unsigned i = 0; i < I; ++i)
		CounterPat[i] = _solver->getValue(_PiList[i]->GetVar());
	_PiPat.push_back(PiPattern(CounterPat));
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/
