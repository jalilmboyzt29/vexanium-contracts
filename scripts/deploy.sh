#!/bin/bash

CONTRACT_NAME="sendvexjalil"
CONTRACT_ACCOUNT="sendvexjalil"
WASM_FILE="../build/${CONTRACT_NAME}/${CONTRACT_NAME}.wasm"
ABI_FILE="../build/${CONTRACT_NAME}/${CONTRACT_NAME}.abi"

echo "Deploying contract ${CONTRACT_NAME}..."

# Set contract
cleos set contract ${CONTRACT_ACCOUNT} ../build/${CONTRACT_NAME} ${WASM_FILE} ${ABI_FILE} -p ${CONTRACT_ACCOUNT}@active

echo "Contract deployed successfully!"
