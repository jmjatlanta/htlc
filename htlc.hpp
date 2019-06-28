#pragma once

#include <string>
#include <eosiolib/crypto.hpp>
#include <eosiolib/time.hpp>
#include <eosio.token/eosio.token.hpp>

class [[eosio::contract("htlc")]] htlc : public eosio::contract
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
      uint64_t build(eosio::name sender, eosio::name receiver, eosio::asset token, 
            eosio::checksum256 hashlock, eosio::time_point timelock);

      /*****
       * I have the preimage. Send the tokens to the receiver
       */
      [[eosio::action]]
      void withdrawhtlc(uint64_t id, std::string preimage);

      /*****
       * Return the tokens to the sender
       */
      [[eosio::action]]
      void refundhtlc(uint64_t id);

      [[eosio::on_notify("eosio.token::transfer")]]
      void transfer_happened( eosio::name from, eosio::name to, eosio::asset quantity,
            const std::string& memo );

      using transfer_action = eosio::action_wrapper<eosio::name("transfer"), &htlc::transfer_happened>;

   private:

      /*****
       * A table that keeps user balances
       */
      struct [[eosio::table]] htlc_balance
      {
         eosio::name owner;
         std::vector<eosio::asset> balances;

         uint64_t primary_key() const { return owner.value; }

         EOSLIB_SERIALIZE( htlc_balance, (owner)(balances));
      };

      /***
       * persistence record format
       */
      struct [[eosio::table]] htlc_contract
      {
         uint64_t key; // unique key
         eosio::checksum256 id;
         eosio::name sender; // who created the HTLC
         eosio::name receiver; // the destination for the tokens
         eosio::asset token; // the token and quantity
         eosio::checksum256 hashlock; // the hash of the preimage
         eosio::time_point timelock; // when the contract expires and sender can ask for refund
         bool withdrawn; // true if receiver provides the preimage
         bool refunded; // true if sender is refunded
         std::string preimage; /// the preimage provided by the receiver to claim

         uint64_t primary_key() const { return key; }
         eosio::checksum256 by_id() const { return id; }

         htlc_contract() {}

         htlc_contract(eosio::name sender, eosio::name receiver, eosio::asset token,
               eosio::checksum256 hashlock, eosio::time_point timelock)
         {
            this->sender = sender;
            this->receiver = receiver;
            this->token = token;
            this->hashlock = hashlock;
            this->timelock = timelock;
            this->preimage = "";
            this->withdrawn = false;
            this->refunded = false;
            this->id = create_id(this); // unique secondary index
            // NOTE: this->key is set elsewhere
         }

         EOSLIB_SERIALIZE(htlc_contract, 
               (key)
               (sender)
               (receiver)
               (token)
               (hashlock)
               (timelock)
               (withdrawn)
               (refunded)
               (preimage)
               );
      };

      /*****
       * Indexing
       */
      typedef eosio::multi_index<"htlcs"_n, htlc_contract,
            eosio::indexed_by<"id"_n, 
            eosio::const_mem_fun<htlc_contract, eosio::checksum256, 
            &htlc_contract::by_id>>> htlc_index;

      typedef eosio::multi_index<"balances"_n, htlc_balance> balances_index; 

      /****
       * Assists in building the id. This should only be called by the ctor, as some fields
       * must be set to the default for hashing to work correctly
       * 
       * @param in the contract to generate the id from
       * @param the sha256 hash of the contract
       */
      static eosio::checksum256 create_id( htlc_contract* const in)
      {
         return eosio::sha256( reinterpret_cast<char *>(in), sizeof(htlc_contract)  );
      }

      /***
       * Get a contract by its id
       */
      std::shared_ptr<htlc::htlc_contract> get_by_id(eosio::checksum256 id); 

      /***
       * Get a contract by its key
       */
      std::shared_ptr<htlc::htlc_contract> get_by_key(uint64_t id); 
};
