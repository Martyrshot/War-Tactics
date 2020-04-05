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

War Tactics requires v1.9.9-stable [https://github.com/etheruem/go-ethereum/releases/tag/v1.9.9
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

brew tap ethereum/ethereum
brew install solidity
