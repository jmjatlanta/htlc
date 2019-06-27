#!/bin/bash

DEV_DIR=/Users/JohnJones/Development/cpp
DATADIR=$DEV_DIR/eos/contracts/htlc/blockchain
WALLETDIR=$DEV_DIR/eos/contracts/htlc/eosio-wallet

if [ ! -d $DATADIR ]; then
   mkdir -p $DATADIR;
   mkdir -p $DATADIR/config
   cp ./config.dev.ini $DATADIR/config/config.ini
fi

if [ ! -d $WALLETDIR ]; then
   mkdir -p $WALLETDIR;
   cp ./config.wallet.ini $WALLETDIR/config.ini
fi

# start keosd                                                                                                              
keosd --wallet-dir $WALLETDIR & \
echo $! > $DATADIR"/keosd.pid"
                                                                                                                           
# give it a chance to start                                                                                                
sleep 3                                                                                                                    

# now start nodeos
nodeos -p eosio \
--signature-provider EOS_PUB_DEV_KEY=KEY:EOS_PRIV_DEV_KEY \
--plugin eosio::producer_plugin \
--plugin eosio::chain_api_plugin \
--plugin eosio::http_plugin \
--plugin eosio::history_api_plugin \
--plugin eosio::history_plugin \
--data-dir $DATADIR"/data" \
--blocks-dir $DATADIR"/blocks" \
--config-dir $DATADIR"/config" \
--producer-name eosio \
--http-server-address 127.0.0.1:8888 \
--p2p-listen-endpoint 127.0.0.1:9010 \
--access-control-allow-origin=* \
--contracts-console \
--http-validate-host=false \
--verbose-http-errors \
--enable-stale-production \
--p2p-peer-address localhost:9011 \
--p2p-peer-address localhost:9012 \
--p2p-peer-address localhost:9013 \
>> $DATADIR"/nodeos.log" 2>&1 & \
echo $! > $DATADIR"/nodeos.pid"
