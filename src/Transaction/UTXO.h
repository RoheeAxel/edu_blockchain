#pragma once
#include <openssl/evp.h>
#include <stdbool.h>


typedef struct {
    char prev_txid[65];     // ID (hash) of the previous transaction
    int output_index;       // Which output of that transaction
    char signature[256];    // Signature by sender to prove ownership
    char coinbase[65];     // CoinBase transaction data
} TxInput;

typedef struct {
    char recipient[65];     // Usually a public key or its hash
    int amount;
} TxOutput;

TxInput *TxInput__CreateCoinbaseTransaction(const char *coinbase);
TxInput *TxInput__Create(const char *prev_txid, int output_index);
size_t TxInput__Serialize(TxInput *input, char *output);
void TxInput__Print(TxInput *input);
void TxInput__Sign(TxInput *input, EVP_PKEY *private_key);
bool TxInput__Verify(const TxInput *input, EVP_PKEY *public_key);

TxOutput *TxOutput__Create(const char *recipient, int amount);
size_t TxOutput__Serialize(TxOutput *output, char *data);
void TxOutput__Print(TxOutput *output);