#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include "Transaction.h"
#include "Block.h"


void Transaction__Hash(Transaction *tx, char *output) {
    if (!tx || !output) return;

    // Estimate the max size needed (inputs + outputs)
    size_t estimated_size = len(tx->inputs) * (65 + sizeof(int) + 256) +
                            len(tx->outputs) * (65 + sizeof(int));

    unsigned char *data = malloc(estimated_size);
    if (!data) return;

    size_t offset = 0;

    // Serialize inputs
    for (size_t i = 0; i < len(tx->inputs); i++) {
        TxInput *in = value_at(tx->inputs, i);
        offset += TxInput__Serialize(in, (char *)(data + offset));
    }

    // Serialize outputs
    for (size_t i = 0; i < len(tx->outputs); i++) {
        TxOutput *out = value_at(tx->outputs, i);
        offset += TxOutput__Serialize(out, (char *)(data + offset));
    }

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(data, offset, hash);

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        sprintf(output + (i * 2), "%02x", hash[i]);
    output[64] = '\0'; // null-terminate
    memcpy(tx->txid, output, 65); // Store the hash in the transaction
    free(data);
}

Transaction* Transaction__Create(vector(TxInput) *inputs, vector(TxOutput) *outputs) {
    Transaction *tx = malloc(sizeof(Transaction));
    if (!tx) return NULL;

    tx->inputs = inputs;  // Point to the inputs vector passed
    tx->outputs = outputs; // Point to the outputs vector passed
    return tx;
}

void Transaction__Sign(Transaction *tx, EVP_PKEY *private_key) {
    if (!tx || !private_key) return;

    for (size_t i = 0; i < len(tx->inputs); ++i) {
        TxInput__Sign(value_at(tx->inputs, i), private_key);
    }
}

// bool Transaction__Verify(const Transaction *tx, EVP_PKEY *public_key) {
//     if (!tx || !public_key) return false;

//     for (size_t i = 0; i < len(tx->inputs); ++i) {
//         if (!TxInput__Verify(value_at(tx->inputs, i), public_key)) {
//             return false;  // If any input fails verification, the transaction is invalid
//         }
//     }

//     return true;
// }


bool Transaction__Verify(Blockchain *chain, Transaction *tx) {
    if (!tx) return false;

    int total_input_amount = 0;
    int total_output_amount = 0;

    for (size_t i = 0; i < len(tx->inputs); ++i) {

        if (value_at(tx->inputs, i)->signature == NULL) {
            printf("❌ Error: Input %zu signature is NULL!\n", i);
            return false;
        }
        Transaction *prev_tx = Blockchain__FindTransactionByID(chain, value_at(tx->inputs, i)->prev_txid);
        if (prev_tx == NULL) {
            printf("❌ Error: Input %zu prev_txid with id %s not found in blockchain!\n", i, value_at(tx->inputs, i)->prev_txid);
            return false;
        }

        int index = value_at(tx->inputs, i)->output_index;

        if (len(prev_tx->outputs) <= index) {
            printf("❌ Error: Input %zu output_index out of bounds!\n", i);
            return false;
        }


        TxOutput *prev_output = value_at(prev_tx->outputs, index);
        if (prev_output == NULL) {
            printf("❌ Error: Input %zu prev_output is NULL!\n", i);
            return false;
        }
        // TODO check if it the same guy
        printf("I %lu\n", i);
        total_input_amount += prev_output->amount;
    }
    for (size_t i = 0; i < len(tx->outputs); ++i) {
        if (value_at(tx->outputs, i)->recipient == NULL) {
            printf("❌ Error: Output %zu recipient is NULL!\n", i);
            return false;
        }
        if (value_at(tx->outputs, i)->amount <= 0) {
            printf("❌ Error: Output %zu amount is invalid!\n", i);
            return false;
        }
        total_output_amount += value_at(tx->outputs, i)->amount;
    }

    if (total_input_amount < total_output_amount) {
        Block *block = Blockchain__FindBlockByTransactionId(chain, tx->txid);
        if (!Block__VerifyCoinbaseTransaction(block)) {
            printf("❌ Error: Coinbase transaction is invalid!\n");
            return false;
        }
        // printf("❌ Error: Total input amount (%d) is less than total output amount (%d)!\n", total_input_amount, total_output_amount);
        return false;
    }
    return true;
}

void Transaction__Print(Transaction *tx)
{
    if (!tx) return;

    printf("Transaction:\n");
    printf("  Inputs:\n");
    for (size_t i = 0; i < len(tx->inputs); ++i) {
        TxInput *input = value_at(tx->inputs, i);
        TxInput__Print(input);
    }

    printf("  Outputs:\n");
    for (size_t i = 0; i < len(tx->outputs); ++i) {
        TxOutput *output = value_at(tx->outputs, i);
        TxOutput__Print(output);
    }
    printf("\n");
    char hash[65];
    Transaction__Hash(tx, hash);
    printf("Transaction Hash: %s\n", hash);
}
