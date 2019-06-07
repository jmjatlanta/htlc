#include <htlc.hpp>
#include <eosio/crypto.hpp>
#include <eosio/system.hpp>

uint64_t htlc::create(eosio::name sender, eosio::name receiver, eosio::asset token, 
      eosio::checksum256 hashlock, eosio::time_point timelock)
{
   require_auth(sender);
   htlc_index htlcs(get_self(), get_first_receiver().value);
   htlc_contract htlc(sender, receiver, token, hashlock, timelock);
   // make sure the receiver exists
   assert( is_account( receiver ) );
   // make sure the sender has the funds
   const auto bal = eosio::token::get_balance("eosio.token"_n, sender, token.symbol.code());
   eosio::check(token.amount < bal.amount, "Insuffiient Funds");
   // build the record
   uint64_t id = htlcs.available_primary_key();
   htlcs.emplace(sender, [&](auto& row) {
      row.id = id;
      row.sender = htlc.sender;
      row.receiver = htlc.receiver;
      row.token = htlc.token;
      row.hashlock = htlc.hashlock;
      row.timelock = htlc.timelock;
      row.withdrawn = htlc.withdrawn;
      row.refunded = htlc.refunded;
      row.preimage = htlc.preimage;
   });
   // Hold funds in this contract
   eosio::action( eosio::permission_level{sender, "active"_n},
         "eosio.token"_n, "transfer"_n,
         std::make_tuple(_self, "eosio.token"_n, token, std::string("Held in HTLC")));
   return id;
}

void htlc::withdraw(uint64_t key, std::string preimage)
{
   htlc_index htlcs(get_self(), get_first_receiver().value);
   auto iterator = htlcs.find(key);
   // basic checks
   eosio::check( iterator != htlcs.end(), "HTLC not found");
   eosio::check( !(*iterator).withdrawn, "Tokens from this HTLC have already been withdrawn" );
   eosio::check( !(*iterator).refunded, "Tokens from this HTLC have already been refunded");
   eosio::check( (*iterator).timelock < eosio::current_block_time(), "HTLC timelock expired");
   eosio::checksum256 passed_in_hash = eosio::sha256(preimage.c_str(), preimage.length());
   eosio::check( (*iterator).hashlock == passed_in_hash, "Preimage mismatch");
   // TODO: all looks good, do the transfer

 }

void htlc::refund(uint64_t key)
{
   htlc_index htlcs(get_self(), get_first_receiver().value);
   auto iterator = htlcs.find(key);
   // basic checks
   eosio::check( iterator != htlcs.end(), "HTLC not found");
   eosio::check( !(*iterator).withdrawn, "Tokens from this HTLC have already been withdrawn" );
   eosio::check( !(*iterator).refunded, "Tokens from this HTLC have already been refunded");
   eosio::check( (*iterator).timelock >= eosio::current_block_time(), "HTLC timelock expired");
   // TODO: all looks good, do the refund
}


