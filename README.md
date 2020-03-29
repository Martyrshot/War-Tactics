# 434-project

## Dependencies

This list is a work in progress and more will be added

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


#### Solidity Compiler (solc)

##### Ubuntu/Debian

`sudo apt update && sudo apt install solc`
