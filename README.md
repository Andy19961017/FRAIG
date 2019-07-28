Functionally Reduced And-Inverter Graph (FRAIG)
===
This program is an electronic design automation (EDA) tool written by C++ that can reduce the size of logical circuits automatically while remaining the functionality. It can perform the following functions:  
1.Parses the logical circuit description file in the AIGER format.  
2.Uses hash table to detect structurally equivalent signals in a circuit and remove redundant gates.  
3.Performs Boolean logic simulations and identify "functionally equivalent candidate pairs" (gates having the same output pattern throughout the entire simulation) in the circuit.  
4.Calls Boolean Satisfiability (SAT) solver to prove the functional equivalence, checking if each of the "functionally equivalent candidate pairs" are really functionally equivalent or just have the same output pattern by chance.  

For detailed description, please refer to [this](https://github.com/Andy19961017/FRAIG/blob/master/FraigProject.pdf).  

## Supported Platform:  
  Linux / Ubuntu 5.4.0-6 ubuntu1~16.04.4  
  Mac / Homebrew GCC 7.2.0  
  
## Ussage:  
  For Linux
  ```
  make linux
  make
  ./fraig
  ```
  For Mac
  ```
  make mac
  make
  ./fraig
  ```
  
## Reference:
1.http://fmv.jku.at/aiger/  
2.http://minisat.se/  

