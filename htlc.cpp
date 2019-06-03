#include <htlc.hpp>

uint64_t htlc::create(eosio::name sender, eosio::name receiver, eosio::asset token, 
      eosio::checksum256 hashlock, uint64_t timelock)
{
   require_auth(sender);
   htlc_index htlcs(get_self(), get_first_receiver().value);
   htlc_contract htlc(sender, receiver, token, hashlock, timelock);
   uint64_t key = htlcs.available_primary_key();
   htlcs.emplace(sender, [&](auto& row) {
      row.key = key;
      row.sender = htlc.sender;
      row.receiver = htlc.receiver;
      row.token = htlc.token;
      row.hashlock = htlc.hashlock;
      row.timelock = htlc.timelock;
      row.withdrawn = htlc.withdrawn;
      row.refunded = htlc.refunded;
      row.preimage = htlc.preimage;
   });
   return key;
}

void htlc::withdraw(std::string preimage)
{

}

void htlc::refund()
{

}


