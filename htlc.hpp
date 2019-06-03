#pragma once

#include <string>
#include <eosio.token/eosio.token.hpp>

class [[eosio::contract]] htlc : public eosio::contract
{
   public:
      /*****
       * default constructor
       */
      htlc(eosio::name receiver, eosio::name code, eosio::datastream<const char*> ds)
            :contract(receiver, code, ds) {}

      /****
       * Create a new HTLC
       */
      [[eosio::action]]
      uint64_t create(eosio::name sender, eosio::name receiver, eosio::asset token, 
            eosio::checksum256 hashlock, eosio::time_point timelock);

      /*****
       * I have the preimage. Send the tokens to the receiver
       */
      [[eosio::action]]
      void withdraw(uint64_t key, std::string preimage);

      /*****
       * Return the tokens to the sender
       */
      [[eosio::action]]
      void refund(uint64_t);

   private:
      struct [[eosio::table]] htlc_contract
      {
         uint64_t key; // unique key
         eosio::checksum256 id; // unique key to prevent duplicates
         eosio::name sender; // who created the HTLC
         eosio::name receiver; // the destination for the tokens
         eosio::asset token; // the token and quantity
         eosio::checksum256 hashlock; // the hash of the preimage
         eosio::time_point timelock; // when the contract expires and sender can ask for refund
         bool withdrawn; // true if receiver provides the preimage
         bool refunded; // true if sender is refunded
         std::string preimage; /// the preimage provided by the receiver to claim

         uint64_t primary_key() const { return key; }

         htlc_contract() {}
         htlc_contract(eosio::name sender, eosio::name receiver, eosio::asset token,
               eosio::checksum256 hashlock, eosio::time_point timelock)
         {
            this->key = 0;
            this->sender = sender;
            this->receiver = receiver;
            this->token = token;
            this->hashlock = hashlock;
            this->timelock = timelock;
            this->preimage = "";
            this->withdrawn = false;
            this->refunded = false;
            //TODO: Make a unique secondary index with a (sha256?) hash to prevent duplicates
         }
      };

      typedef eosio::multi_index<"htlcs"_n, htlc_contract> htlc_index;
};
