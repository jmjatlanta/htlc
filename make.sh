#!/bin/bash

eosio-cpp -abigen -o htlc.wasm htlc.cpp -I../eosio.contracts/contracts/eosio.token/include -I. -I../../../boost169/include
