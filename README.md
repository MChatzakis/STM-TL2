# Software Transactional Memory implementing Transactional Locking 2 (TL2) algorithm.
This repository contains an implementation of a Software Transactional Memory according to the description of Transactional Locking 2 (TL2) algorithm [Dice et. al](https://dcl.epfl.ch/site/_media/education/4.pdf) using C11 atomics.

It was developed by Manos Chatzakis (emmanouil.chatzakis@epfl.ch) at EPFL in December 2023.

## Overview
The implementation of TL2 manages to outperform the corresponding coarse-grained pthread lock implementation of transactional programming, achieving approximately 2.5x speedup. 

## Compile and Run
This library can be included in any C/C++ project. The binary of the library can be generated with the makefile provided
```sh
make build # Will create the tm.c.o file to be included in subsequent compilations.
```
The above command compile the library.
To clean the produced files, use
```sh
make clean
```

## About
This project was developed for the Concurrent Computing course of EPFL.