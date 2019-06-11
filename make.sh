#!/bin/bash

eosio-cpp -abigen -o eos_htlc.wasm eos_htlc.cpp -I../eosio.contracts/contracts/eosio.token/include -I. -I../../../boost169/include
