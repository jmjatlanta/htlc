#!/bin/bash

eosio-cpp -abigen -o htlc.wasm htlc.cpp -I../eosio.contracts/eosio.token/include -I. -I../../../boost169/include
