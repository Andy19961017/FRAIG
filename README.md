Functionally Reduced And-Inverter Graph (FRAIG)
===
This program is an electronic design automation (EDA) tool that can reduce the size of logical circuits automatically. It can perform the following functions:

1.Parses the logical circuit description file in the AIGER format.

2.Uses hash to detect structurally equivalent signals in a circuit.

3.Performs Boolean logic simulations and identify functionally equivalent candidate pairs in the circuit as “functionally equivalent candidate (FEC) pair”.

4.Calls Boolean Satisfiability (SAT) solver to prove the functional equivalence.


Supported Platform:
  Linux / Ubuntu 5.4.0-6ubuntu1~16.04.4
  Mac / Homebrew GCC 7.2.0
  
Ussage:

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
  
