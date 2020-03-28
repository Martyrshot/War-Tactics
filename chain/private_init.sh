#!/bin/bash

FILEPATH=`pwd`

geth init --datadir "${FILEPATH}/chaindata" "${FILEPATH}/genesis.json"
