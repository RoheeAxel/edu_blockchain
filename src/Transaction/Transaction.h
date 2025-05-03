#pragma once
#include <openssl/evp.h>
#include <stdbool.h>
#include "UTXO.h"
#include "vector.h"

init_vector(TxInput);
init_vector(TxOutput);

// #define SIZE_BASE_TRANSACTION (256 + 256 + sizeof(float))

typedef struct {
    vector(TxInput) *inputs;
    vector(TxOutput) *outputs;
    char txid[65];
} Transaction;

typedef struct Blockchain Blockchain;
typedef struct Block Block;

// Blockchain function
Transaction *Blockchain__FindTransactionByID(Blockchain *chain, const char *txid);
Block *Blockchain__FindBlockByTransactionId(Blockchain *chain, const char *txid);
bool Block__VerifyCoinbaseTransaction(Block *block);


Transaction* Transaction__Create(vector(TxInput) *inputs, vector(TxOutput) *outputs);
bool Transaction__Verify(Blockchain *chain, Transaction *transaction);
void Transaction__Sign(Transaction *transaction, EVP_PKEY *private_key);
void Transaction__Print(Transaction *transaction);
void Transaction__Hash(Transaction *tx, char *output);

