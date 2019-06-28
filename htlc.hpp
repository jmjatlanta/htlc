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

      /*****
       * Review details of an HTLC
       * @param id the htlc identifier
       */
      [[eosio::action]]
      void reviewhtlc(uint64_t id);

      /****
       * Query for balances of an account
       * @param acct the account to display
       */
      [[eosio::action]]
      void balances(const eosio::name& acct);

      /****
       * Called when a transfer is made into the contract
       */
      [[eosio::on_notify("eosio.token::transfer")]]
      void transfer_happened(const eosio::name& from, const eosio::name& to, 
            const eosio::asset& quantity, const std::string& memo );
      using transfer_action = eosio::action_wrapper<eosio::name("transfer"), &htlc::transfer_happened>;

   private:

      /*****
       * A table that keeps user balances
       */
      struct [[eosio::table]] htlc_balance
      {
         eosio::asset token;

         uint64_t primary_key() const { return token.symbol.raw(); }

         EOSLIB_SERIALIZE( htlc_balance, (token));
      };

      /*****
       * Retrieve the balance from the htlc_balance table
       * @param acct the account to query
       * @param token the token to check
       * @returns the balance of the given token
       */
      eosio::asset get_balance(const eosio::name& acct, const eosio::asset& token);

      /****
       * Remove some tokens from the account balance of htlc_balance table
       * @param acct the account
       * @param token the token and amount to remove
       * @returns true on success, false otherwise
       */
      bool withdraw_balance(const eosio::name& acct, const eosio::asset& token);

      /***
       * persistence of the HTLC
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
         bool funded;
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
            this->funded = false;
            this->withdrawn = false;
            this->refunded = false;
            this->id = eosio::sha256( reinterpret_cast<char *>(this), sizeof(htlc_contract)  ); // unique secondary index
         }

         EOSLIB_SERIALIZE(htlc_contract, 
               (key)
               (sender)
               (receiver)
               (token)
               (hashlock)
               (timelock)
               (funded)
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

      /***
       * Get an HTLC contract by its id
       */
      std::shared_ptr<htlc::htlc_contract> get_by_id(eosio::checksum256 id); 

      /***
       * Get an HTLC contract by its key
       */
      std::shared_ptr<htlc::htlc_contract> get_by_key(uint64_t id); 

      static double to_real(const eosio::asset& in);

};
