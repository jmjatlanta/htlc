#include <htlc.hpp>
#include <eosiolib/action.h>
#include <eosiolib/system.hpp>

void htlc::transfer_happened( eosio::name from, eosio::name to, eosio::asset quantity,
      const std::string& memo )
{
   // find the account
   balances_index balances(get_self(), eosio::contract::get_code().value);
   auto itr = balances.find(from.value);
   if (itr != balances.end())
   {
      // add the new account
      balances.emplace(get_self(), [&](auto& row)
      {
         row.owner = from;
         std::vector<eosio::asset> assets;
         row.balances = std::vector<eosio::asset>();
         row.balances.push_back(quantity);
      });
   } 
   else
   {
      // account exists, look for the symbol
      htlc_balance  bal = *itr;
      bool found_symbol = false;
      for(auto b : bal.balances)
      {
         if (b.symbol == quantity.symbol)
         {
            found_symbol = true;
            b += quantity;
            break;
         }
      }
      if (!found_symbol)
      {
         bal.balances.push_back(quantity);
      }
      balances.modify(itr, _self, [&](auto& row)
      {
         row.balances = bal.balances;
      });
   } 
}

uint64_t htlc::build(eosio::name sender, eosio::name receiver, eosio::asset token, 
      eosio::checksum256 hashlock, eosio::time_point timelock)
{
   require_auth(sender);
   htlc_index htlcs(get_self(), eosio::contract::get_code().value);
   htlc_contract htlc(sender, receiver, token, hashlock, timelock);
   // make sure an htlc with this hash does not already exist
   std::shared_ptr<htlc::htlc_contract> old_contract = get_by_id(htlc.id);
   eosio::check(old_contract == nullptr, "Another HTLC generates that hash. Try changing parameters slightly.");
   // make sure the receiver exists
   assert( is_account( receiver ) );
   // make sure the sender has the funds
   const auto bal = eosio::token::get_balance("eosio.token"_n, sender, token.symbol.code());
   eosio::print("My balance: ", bal.amount, " Transaction value: ", token.amount );
   eosio::check(token.amount < bal.amount, "Insuffiient Funds");
   // build the record
   uint64_t key = htlcs.available_primary_key();
   htlcs.emplace(get_self(), [&](auto& row) {
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
   eosio::print("HTLC Contract Key: ", key, " Hash: ", htlc.id);
   // Hold funds in this contract
   eosio::action( eosio::permission_level{sender, "active"_n},
         "eosio.token"_n, "transfer"_n,
         std::make_tuple(sender, _self, token, std::string("Held in HTLC"))).send();
   eosio::print("HTLC Contract Key: ", key, " Hash: ", htlc.id);
   return key;
}

void htlc::withdrawhtlc(uint64_t id, std::string preimage)
{
   htlc_index htlcs(get_self(), eosio::contract::get_code().value);
   auto iterator = htlcs.find(id);
   // basic checks
   eosio::check( iterator != htlcs.end(), "HTLC not found");
   htlc_contract contract = *iterator;
   eosio::check( !contract.withdrawn, "Tokens from this HTLC have already been withdrawn" );
   eosio::check( !contract.refunded, "Tokens from this HTLC have already been refunded");
   eosio::check( contract.timelock.sec_since_epoch() < current_time(), "HTLC timelock expired");
   eosio::checksum256 passed_in_hash = eosio::sha256(preimage.c_str(), preimage.length());
   eosio::check( contract.hashlock == passed_in_hash, "Preimage mismatch");
   // update the contract
   htlcs.modify(iterator, get_self(), [&](auto& row)
      {
         row.withdrawn = true;
      });
   // all looks good, do the transfer
   eosio::action( eosio::permission_level{_self, "active"_n},
         "eosio.token"_n, "transfer"_n,
         std::make_tuple(contract.receiver, "eosio.token"_n, contract.token, 
               std::string("Withdrawn from HTLC"))).send();
 }

void htlc::refundhtlc(uint64_t id)
{
   htlc_index htlcs(get_self(), eosio::contract::get_code().value);
   auto iterator = htlcs.find(id);
   // basic checks
   eosio::check( iterator != htlcs.end(), "HTLC not found");
   htlc_contract contract = *iterator;
   eosio::check( !contract.withdrawn, "Tokens from this HTLC have already been withdrawn" );
   eosio::check( !contract.refunded, "Tokens from this HTLC have already been refunded");
   eosio::check( contract.timelock.sec_since_epoch() >= current_time(), "HTLC timelock has not expired");
   // update the contract
   htlcs.modify(iterator, get_self(), [&](auto& row)
      {
         row.refunded = true;
      });   // all looks good, do the refund
   eosio::action( eosio::permission_level{_self, "active"_n},
         "eosio.token"_n, "transfer"_n,
         std::make_tuple(contract.sender, "eosio.token"_n, contract.token, 
               std::string("Refunded from HTLC"))).send();
}

std::shared_ptr<htlc::htlc_contract> htlc::get_by_id(eosio::checksum256 id)
{
   htlc_index htlcs(get_self(), eosio::contract::get_code().value);
   auto id_index = htlcs.get_index<"id"_n>();
   auto iterator = id_index.find(id);
   if ( iterator == id_index.end() )
      return nullptr;
   // make a copy to keep it around
   return std::shared_ptr<htlc::htlc_contract>( new htlc_contract(*iterator));
}

std::shared_ptr<htlc::htlc_contract> htlc::get_by_key(uint64_t id)
{
   htlc_index htlcs(get_self(), eosio::contract::get_code().value);

   auto iterator = htlcs.find(id);
   if ( iterator == htlcs.end() )
      return nullptr;
   // make a copy to keep it around
   return std::shared_ptr<htlc::htlc_contract>( new htlc_contract(*iterator));
}


