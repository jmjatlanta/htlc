#pragma once

#include <string>
#include <eosio.token/eosio.token.hpp>

class [[eosio::contract]] htlc : public eosio::contract
{
   public:
      /*****
       * Constructor
       */
      htlc(eosio::name receiver, eosio::name code, eosio::datastream<const char*> ds)
            :contract(receiver, code, ds)
      {

      }

      /****
       * Create a new HTLC
       */
      [[eosio::action]]
      void create(eosio::name receiver, eosio::asset token, 
            eosio::checksum256 hashlock, uint64_t timelock);

      /*****
       * I have the preimage. Send the tokens to the receiver
       */
      [[eosio::action]]
      void withdraw(std::string preimage);

      /*****
       * Return the tokens to the sender
       */
      [[eosio::action]]
      void refund();

   private:
      struct [[eosio::table]] htlc_contract
      {
         eosio::checksum256 key;
         eosio::name sender;
         eosio::name receiver;
         eosio::asset token;
         eosio::checksum256 hashlock;
         uint64_t timelock;
         bool witdrawn;
         bool refunded;
         std::string preimage;

         eosio::checksum256 primary_key() const { return key; }
      };

      typedef eosio::multi_index<"htlcs"_n, htlc_contract> htlc_index;
};
