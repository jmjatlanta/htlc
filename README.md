# eos_htlc
Create Hashed Time Lock Contracts on the EOS blockchain

Note: This version uses deprecated methods. Once I figure out how to upgrade my mac to more recent versions of eos, I will fix it.

Tutorial on how to use HTLC on your own EOS testnet

Prerequisites:

- A running keosd (the EOS wallet)
- A running nodeos (an EOS testnet node)
- The eosio.contracts repository compiled (see https://github.com/EOSIO/eosio.contracts)

Firstly we must create a wallet

`$ cleos wallet create —to-console`

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

`$ git clone https://github.com/jmjatlanta/eos_htlc`

Compile the htlc contract with the following command (NOTE: The BOOST_ROOT environment variable must be set correctly):

`$ eosio-cpp -abigen -o htlc.wasm htlc.cpp -I../eosio.contracts/eosio.token/include -I. -I$BOOST_ROOT/include`

- TODO: Make the above more flexible. CMake would be great.

With the contract compiled, you can now deploy the contract. We will first create a user that will control the contract. To do that, we will create a new key and import it into our wallet:

```
$ cleos create key —to-console
Private key: 5JxXcF9p7Ncvt37MFA8nWQqaoGnD3dGVmauQjt5L72zx5Mdp3xk
Public key: EOS69m3cpc3ABwZBqa25KDi4qJrKS16azR2sCV4XZ5WtEbndVYaJ1
$ cleos wallet import —private-key 5JxXcF9p7Ncvt37MFA8nWQqaoGnD3dGVmauQjt5L72zx5Mdp3xk
$ cleos create account eosio eosio.htlc EOS69m3cpc3ABwZBqa25KDi4qJrKS16azR2sCV4XZ5WtEbndVYaJ1
```

And deploy the contract

`$ cleos set contract eosio.htlc eosio_htlc -p eosio.htlc@active`

Now we can test it out.
