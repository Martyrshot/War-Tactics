#!/bin/bash

CREDENTIALS=1


FILEPATH=`pwd`

geth \
	--unlock `cat "${FILEPATH}/addr${CREDENTIALS}"` \
	--password "${FILEPATH}/pw${CREDENTIALS}" \
	--syncmode 'full' \
	--gasprice 0 \
	--cache=3072 \
	--etherbase `cat "${FILEPATH}/addr${CREDENTIALS}"` \
	--mine \
	console \
	--ipcpath "${HOME}/.ethereum/geth.ipc" \
	--datadir "${FILEPATH}/chaindata" \
	--networkid 21811 \
	--port 30303 \
	--targetgaslimit 40000000 \
	--allow-insecure-unlock \
	--rpc
