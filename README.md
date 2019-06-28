# htlc
Create Hashed Time Lock Contracts on the EOS blockchain

Note: This version uses deprecated methods. Once I figure out how to upgrade my mac to more recent versions of eos, I will fix it.

Tutorial on how to use HTLC on your own EOS testnet

Prerequisites:

- A running keosd (the EOS wallet)
- A running nodeos (an EOS testnet node)
- The eosio.contracts repository compiled (see https://github.com/EOSIO/eosio.contracts)

Firstly we must create a wallet

`$ cleos wallet create --to-console`

This will generate a wallet and provide a password. The wallet is now opened and unlocked.

Note: That password will be needed to reopen this wallet. Without that password, you will be unable to reopen the wallet, and will have to start over. Record the password for later use.

Now we must import the private key that will give you control of your testnet that is running.

`cleos wallet import --private-key 5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3`

That will import the private key for the public key EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV

Now we will need to install a few default system contracts. From the build directory of your eosio.contracts directory, install the eosio.system and eosio.bios contract by following these commands:

`$ cleos set contract eosio contracts/eosio.system -p eosio@active`

`$ cleos set contract eosio contracts/eosio.bios -p eosio@active`

Next, we will install the eosio.token contract Start by creating an account called “eosio.token”, who will be the owner of the contract.

```
$cleos create key --to-console
Private key: 5Kh3sX6Lv4WsKWnwngUJbKhW9dtr8Pr7hG6CFiKCmiKg9QxPhBv
Public key: EOS8gUtB8M2vQU9NanxBmCNqYRJVXmt6MwDBk2LgDmV9FZRXoAoPN
```

Now import that private key into your wallet

`$ cleos wallet import --private-key 5Kh3sX6Lv4WsKWnwngUJbKhW9dtr8Pr7hG6CFiKCmiKg9QxPhBv`

And then link that key to an account called “eosio.token”

`$ cleos create account eosio eosio.token EOS8gUtB8M2vQU9NanxBmCNqYRJVXmt6MwDBk2LgDmV9FZRXoAoPN EOS8gUtB8M2vQU9NanxBmCNqYRJVXmt6MwDBk2LgDmV9FZRXoAoPN`

Note: That creates an account “eosio.token” with the same Owner and Active key. That may be fine for testing, but should not be done in a production environment.

Now deploy the contract:

`$ cleos set contract eosio.token contracts/eosio.token -p eosio.token@active`

You are almost there. Now we will deploy the htlc contract. Navigate to where you would like to clone the htlc contract repository, and run the git clone command:

`$ git clone https://github.com/jmjatlanta/htlc`

Compile the htlc contract with the following command (NOTE: The BOOST_ROOT environment variable must be set correctly):

`$ eosio-cpp -abigen -o htlc.wasm htlc.cpp -I../eosio.contracts/contracts/eosio.token/include -I. -I$BOOST_ROOT/include`

- TODO: Make the above more flexible. CMake would be great.

With the contract compiled, you can now deploy the contract. We will first create a user that will control the contract. To do that, we will create a new key and import it into our wallet:

```
$ cleos create key --to-console
Private key: 5JxXcF9p7Ncvt37MFA8nWQqaoGnD3dGVmauQjt5L72zx5Mdp3xk
Public key: EOS69m3cpc3ABwZBqa25KDi4qJrKS16azR2sCV4XZ5WtEbndVYaJ1
$ cleos wallet import --private-key 5JxXcF9p7Ncvt37MFA8nWQqaoGnD3dGVmauQjt5L72zx5Mdp3xk
$ cleos create account eosio htlc EOS69m3cpc3ABwZBqa25KDi4qJrKS16azR2sCV4XZ5WtEbndVYaJ1
```

And deploy the contract

```
cleos set contract htlc htlc -p htlc@active
cleos set account permission htlc active --add-code
```

Now we can test it out.

If you haven't already followed the token tutorial, follow these steps to prepare your test environment:

create a private/public key pair that we can use for both Alice and Bob (NOTE: it is a bad idea to share private keys like this, but we are only in a tutorial):

`cleos create key --to-console`

The output for me was:

```
Private key: 5JkdVmBrtvNvdCNjw2B3FbSBCiFcPvcuHnUHJ4CzR8Re3wgXp8K
Public key: EOS7nDGfE7EYxGYs2GKEnMKUfGNZVRPivPnXN8sefdynb7q43jxb8
```

And we will now import the private key into the wallet, and use the public key to create users `alice` and `bob`.

```
cleos wallet import --private-key 5JkdVmBrtvNvdCNjw2B3FbSBCiFcPvcuHnUHJ4CzR8Re3wgXp8K
cleos create account eosio bob EOS7nDGfE7EYxGYs2GKEnMKUfGNZVRPivPnXN8sefdynb7q43jxb8
cleos create account eosio alice EOS7nDGfE7EYxGYs2GKEnMKUfGNZVRPivPnXN8sefdynb7q43jxb8
```

now create the token `SYS`:

```
cleos push action eosio.token create '[ "eosio", "1000000000.0000 SYS"]' -p eosio.token@active
cleos push action eosio.token issue '[ "alice", "100.0000 SYS", "memo" ]' -p eosio@active
cleos push action eosio.token issue '[ "bob", "50.0000 SYS", "memo" ]' -p eosio@active
```

We can verify that everything is okay by checking the balances in the accounts:

```
cleos get currency balance eosio.token alice SYS
cleos get currency balance eosio.token bob SYS
```

If we did everything correctly, Alice should have 100 SYS, and bob should have 50 SYS.

Testing HTLC:

Alice wants a secret password for an online game, and is willing to pay for it. Bob has the password, and is willing to sell it for 12 SYS. Alice is willing to pay 12 SYS, but does not trust that Bob will give the password if she pays him first.

Alice and Bob agree to write and execute an HTLC to handle the exchange of payment and information.

Bob hashes the game's password using the SHA256 algorithm. The hash is `5899575803417E3356A133C51EFFF8314C0D3D7A52F37472F90B1DCE5288525B`. He shares that hash with Alice.

Alice locks 12 SYS in an HTLC contract that will transfer that amount to Bob if he provides the password that matches the hash. 

```
cleos push action eosio.token transfer '["alice", "htlc", "12.0000 SYS", "For the future"]' -p alice@active
cleos push action htlc build '["alice", "bob", "12.0000 SYS" "5899575803417E3356A133C51EFFF8314C0D3D7A52F37472F90B1DCE5288525B", "2019-07-04T00:00:00"]' -p alice@active
```

That action provides an identifier that Alice can give Bob so he can easily look at the contract. In this case, the identifier was "3". Bob can claim the funds by passing in the identifier and the password.

`cleos push action htlc withdrawhtlc '["3", "SuperSecret"]' -p bob`

If Bob chooses not to give away the password, Alice can retrieve her 12 SYS after midnight on the 4th of July.

`cleos push action htlc refundhtlc '["3"]' -p alice`

To review the contract, anyone can call reviewhtlc:

`cleos push action htlc reviewhtlc '["3"]' -p alice`

ToDo: 
- [ ] Have a way to browse by from, to, hash
- [ ] Allow for other hash algos ( i.e. RIPEMD160, HASH160 )
- [ ] Provide a way for refunding a deposit (i.e. HTLC never created)
- [ ] Better time formatting in reviewhtlc
- [ ] Security and performance review
