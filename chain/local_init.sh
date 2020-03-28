#!/bin/bash

FILEPATH=`pwd`

geth init --datadir "${FILEPATH}/testchaindata" "${FILEPATH}/genesis.local.json"
