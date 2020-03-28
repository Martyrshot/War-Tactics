#!/bin/bash

FILEPATH=`pwd`


geth \
	--unlock `cat "${FILEPATH}/addr1"` \
	--password "${FILEPATH}/pw1" \
	--syncmode 'full' \
	--gasprice 0 \
	--cache=3072 \
	--etherbase `cat "${FILEPATH}/addr1"` \
	--mine \
	console \
	--ipcpath "${HOME}/.ethereum/geth.ipc" \
	--datadir "${FILEPATH}/testchaindata" \
	--networkid 21811 \
	--port 30303 \
	--allow-insecure-unlock \
	--netrestrict 127.0.0.1/0
