#include <eos_htlc.hpp>
#include <eosiolib/action.h>
#include <eosiolib/system.hpp>

uint64_t eos_htlc::deposit(eosio::name sender, eosio::name receiver, eosio::asset token, 
      eosio::checksum256 hashlock, eosio::time_point timelock)
{
   require_auth(sender);
   htlc_index htlcs(get_self(), eosio::contract::get_code().value);
   htlc_contract htlc(sender, receiver, token, hashlock, timelock);
   // make sure an htlc with this hash does not already exist
   std::shared_ptr<eos_htlc::htlc_contract> old_contract = get_by_id(htlc.id);
   eosio::check(old_contract == nullptr, "Another HTLC generates that hash. Try changing parameters slightly.");
   // make sure the receiver exists
   assert( is_account( receiver ) );
   // make sure the sender has the funds
   const auto bal = eosio::token::get_balance("eosio.token"_n, sender, token.symbol.code());
   eosio::check(token.amount < bal.amount, "Insuffiient Funds");
   // build the record
   uint64_t key = htlcs.available_primary_key();
   htlcs.emplace(sender, [&](auto& row) {
      row.key = key;
      row.id = htlc.id;
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
   return key;
}

void eos_htlc::withdraw(uint64_t id, std::string preimage)
{
   std::shared_ptr<htlc_contract> contract = get_by_key(id);
   // basic checks
   eosio::check( contract != nullptr, "HTLC not found");
   eosio::check( !contract->withdrawn, "Tokens from this HTLC have already been withdrawn" );
   eosio::check( !contract->refunded, "Tokens from this HTLC have already been refunded");
   eosio::check( contract->timelock.sec_since_epoch() < current_time(), "HTLC timelock expired");
   eosio::checksum256 passed_in_hash = eosio::sha256(preimage.c_str(), preimage.length());
   eosio::check( contract->hashlock == passed_in_hash, "Preimage mismatch");
   // all looks good, do the transfer
   eosio::action( eosio::permission_level{_self, "active"_n},
         "eosio.token"_n, "transfer"_n,
         std::make_tuple(contract->receiver, "eosio.token"_n, contract->token, 
               std::string("Withdrawn from HTLC")));
 }

void eos_htlc::refund(uint64_t id)
{
   std::shared_ptr<eos_htlc::htlc_contract> contract = get_by_key(id);
   // basic checks
   eosio::check( contract != nullptr, "HTLC not found");
   eosio::check( !contract->withdrawn, "Tokens from this HTLC have already been withdrawn" );
   eosio::check( !contract->refunded, "Tokens from this HTLC have already been refunded");
   eosio::check( contract->timelock.sec_since_epoch() >= current_time(), "HTLC timelock has not expired");
   // all looks good, do the refund
   eosio::action( eosio::permission_level{_self, "active"_n},
         "eosio.token"_n, "transfer"_n,
         std::make_tuple(contract->sender, "eosio.token"_n, contract->token, 
               std::string("Refunded from HTLC")));
}

std::shared_ptr<eos_htlc::htlc_contract> eos_htlc::get_by_id(eosio::checksum256 id)
{
   htlc_index htlcs(get_self(), eosio::contract::get_code().value);
   auto id_index = htlcs.get_index<"id"_n>();
   auto iterator = id_index.find(id);
   if ( iterator == id_index.end() )
      return nullptr;
   // make a copy to keep it around
   return std::shared_ptr<eos_htlc::htlc_contract>( new htlc_contract(*iterator));
}

std::shared_ptr<eos_htlc::htlc_contract> eos_htlc::get_by_key(uint64_t id)
{
   htlc_index htlcs(get_self(), eosio::contract::get_code().value);

   auto iterator = htlcs.find(id);
   if ( iterator == htlcs.end() )
      return nullptr;
   // make a copy to keep it around
   return std::shared_ptr<eos_htlc::htlc_contract>( new htlc_contract(*iterator));
}


