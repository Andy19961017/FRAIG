/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void
CirMgr::sweep()
{
	CirGate::setGlobRef();
	setDFStoRef();
	for (unsigned i = 0, n=_totalList.size(); i < n; ++i)
	{
		if (_totalList[i]){
			if (_totalList[i]->isGlobRef()) continue;
			_totalList[i]->sweep();
		}
	}
    setDFSList();
}

void CirMgr::delGate(const unsigned& id)
{
	_totalList[id]=0;
}


// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize()
{
	CirGate::setGlobRef();
  	for (unsigned i = 0, n=_DFSList.size(); i < n; ++i) _DFSList[i]->optimize();  	
  	setDFSList();
}

void CirMgr::setDFStoRef() const
{
	for (unsigned i = 0, n=_DFSList.size(); i < n; ++i)
		_DFSList[i]->setRef();
}


/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
