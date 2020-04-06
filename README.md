# 434-project

## Dependencies

This list is a work in progress and more will be added

This repository contains submodules from other repositories. After cloning, use
`git submodule update --init --recursive --depth=1`

#### ethabi
Compile and install the ethabi binary. It MUST use only this one specific commit from the project as listed. This is because of a bug was fixed on one commit, then the commit following that one changed the CLI fairly radically and broke compatibility.

First install rustc & cargo as directed at [https://www.rust-lang.org/tools/install]

Ensure that the necessary commands to ~/.profile as specified

Then
`cargo install --git https://github.com/paritytech/ethabi.git --rev 7de908fccb2426950dc38a412c35bf1c5b1f6561`

Test ethabi works: `ethabi -h`

#### Boost Library

##### Ubuntu/Debian

`sudo apt update && sudo apt install libboost-all-dev`

##### MacOS

Install the Homebrew package manager from [https://brew.sh]

`brew install boost`

`brew install boost-python`

#### Geth Ethereum Client

War Tactics requires v1.9.9-stable https://github.com/etheruem/go-ethereum/releases/tag/v1.9.9

`git clone https://github.com/ethereum/go-ethereum.git`

`git checkout v1.9.9`

`make`

#### Solidity Compiler (solc)

##### Ubuntu

`sudo apt update && sudo apt install solc`

##### Debian

The Ubuntu based apt repository for Solidity is not compatible with Debian. Use the Snap package manger to install instead.

`sudo apt install -y snapd && sudo snap install solc --edge`
(--edge ensures that you install at least version 0.6.0)

##### MacOS

`brew tap ethereum/ethereum`
`brew install solidity`


## Configuration

This project utilizes numerous dependencies. Please refer to the readme file for a list of dependencies and how to obtain them. Once all dependencies have been installed, some configuration and initialization is required.

Acquire the repository:
```git clone --recurse-submodules git@github.com:Martyrshot/434-project.git
cd 434-project/chain```

Store the directory to an environment variable:
```CHAINDIR=`pwd` ```

This set of instructions will have you setup a private Ethereum network. This requires at least 2 seperate Geth clients to pair with each other to work. Both clients must use a unique address. This repo provides 3 address/password pairs for easy testing.

To select which pair of address/password, set it in the `private_chain.sh` file. Ensure you use a different values for each client you setup.
Edit `private_chain.sh` and change the value of the "CREDENTIALS" environment variable and set it to one of: "1", "2", or "3" (unquoted).

In addition to the above edit, also set the value of the "--ipcpath" flag to:
	If on Mac OS X: `/Users/USERNAME/Library/Ethereum/geth.ipc`, where USERNAME is your home folder
	If on Linux: `/home/USERNAME/.ethereum/geth.ipc`, where USERNAME is your homefolder

```vim ${CHAINDIR}/private_chain.sh```

Edit game.conf
Change "clientAddress" to "Public address of the key" from above"
	If on Mac OS X: Change "ipcPath" to the absolute path: `/Users/USERNAME/Library/Ethereum/geth.ipc`, where USERNAME is your home folder
	If on Linux: Change "ipcPath" to the absolute path: `/home/USERNAME/.ethereum/geth.ipc`, where USERNAME is your homefolder

```vim ${CHAINDIR}/../game.conf```

Now initialize the blockchain.
```./private_init.sh```

## Starting Up

Run geth.
```./private_chain.sh```


You will now need to likely tell each client how to connect to the other client.
On either client, from within Geth:

```admin.nodeInfo.enode```

This will output a string resembling:

> "enode://e367fe669a09dfbb9cc9e86809fcd2175eb3c5de3123a5dbeb7175e27780598f643030762f80735b326c3e134b619ffed7d1d22f72a4805bf9f02030d0d6488b@172.16.1.66:30303"

From the other client, provide this information with the `admin.addPeer()` function, as in the example below:

```admin.addPeer("enode://e367fe669a09dfbb9cc9e86809fcd2175eb3c5de3123a5dbeb7175e27780598f643030762f80735b326c3e134b619ffed7d1d22f72a4805bf9f02030d0d6488b@172.16.1.66:30303")```

After a few moments you should begin to see output indicating that mining occuring every 5 seconds.

In a seperate terminal, navigate to the root of the repository directory.

If you have not yet compiled the game, do so with `make`.

Run the game with `./client`

