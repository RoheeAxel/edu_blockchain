#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/sha.h>
#include "vector.h"
#include "Block.h"
#include "MerkleTree.h"
#include "Blockchain.h"

void Block__Hash(Block *block, char *output) {
    char input[2048];  // Increased buffer size just in case
    size_t offset = 0;

    // Hash basic header info
    offset += snprintf(input + offset, sizeof(input) - offset, "%d%ld%s%d",
                       block->header.index,
                       block->header.timestamp,
                       block->header.previous_hash,
                       *(block->header.nonce));

    for (int i = 0; i < block->transactions->get_size(block->transactions); i++) {
        Transaction *transaction = block->transactions->at(block->transactions, i);
        char txid[65];
        Transaction__Hash(transaction, txid);

        offset += snprintf(input + offset, sizeof(input) - offset, "%s", txid);
        if (offset >= sizeof(input)) {
            fprintf(stderr, "Block__Hash error: input buffer overflow\n");
            break;
        }
    }

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)input, offset, hash);

    // Convert binary hash to hex
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        sprintf(output + (i * 2), "%02x", hash[i]);
    output[64] = '\0';  // Null-terminate
}

Transaction *Blockchain__CreateCoinbaseTransaction(const char *miner_pubkey) {
    vector(TxInput) *inputs = newT(vector, TxInput, 0);  // empty inputs
    vector(TxOutput) *outputs = newT(vector, TxOutput, 0);

    outputs->push_back(outputs, TxOutput__Create(miner_pubkey, BLOCKCHAIN__COINBASE_REWARD));

    return Transaction__Create(inputs, outputs);
}

void Block__Mine(Block *block, char *miner_pubkey) {
    printf("⛏️  Mining block...\n");
    char prefix[65];
    int difficulty = block->header.difficulty;
    memset(prefix, '0', difficulty);
    prefix[difficulty] = '\0';

    block->transactions->push_front(block->transactions, Blockchain__CreateCoinbaseTransaction(miner_pubkey));
    char *merkle_root = compute_merkle_root(block->transactions);
    strcpy(block->header.merkle_root, merkle_root);
    free(merkle_root);
    while (strncmp(block->header.hash, prefix, difficulty) != 0) {
        (*block->header.nonce)++;
        Block__Hash(block, block->header.hash);
    }
    printf("✅ Block mined: %s\n", block->header.hash);
}


bool Block__MeetDifficulty(Block *block) {
    int difficulty = block->header.difficulty;
    char prefix[difficulty + 1];
    memset(prefix, '0', difficulty);
    prefix[difficulty] = '\0';

    return strncmp(block->header.hash, prefix, difficulty) == 0;
}


bool Block__VerifyCoinbaseTransaction(Block *block) {
    if (len(block->transactions) == 0) {
        printf("❌ Error: No transactions in block!\n");
        return false;
    }

    Transaction *coinbase_tx = block->transactions->at(block->transactions, 0);
    if (len(coinbase_tx->inputs) != 0) {
        printf("❌ Error: Coinbase transaction should have no inputs!\n");
        return false;
    }

    return true;
}

bool Block__Verify(Blockchain *chain, Block *block, Block *prev_block)
{
    char hash[65];
    Block__Hash(block, hash);

    if (!Block__VerifyCoinbaseTransaction(block)) {
        printf("❌ Error: Coinbase transaction is invalid!\n");
        return false;
    }

    if (strncmp(block->header.hash, hash, 64) != 0) {
        printf("❌ Error: Block hash mismatch!\n");
        return false;
    }

    if (!prev_block) {
        return true;  // No previous block to verify against
    }

    if (prev_block && strncmp(block->header.previous_hash, prev_block->header.hash, 64) != 0) {
        printf("❌ Error: Previous block hash mismatch!\n");
        return false;
    }

    for (int i = 0; i < len(block->transactions); i++) {
        Transaction *transaction = block->transactions->at(block->transactions, i);
        if (!Transaction__Verify(chain, transaction) && i != 0) {
            printf("❌ Error: Transaction %d is invalid!\n", i);
            return false;
        }
    }


    if (!Block__MeetDifficulty(block)) {
        printf("❌ Error: Block does not meet difficulty!\n");
        return false;
    }

    if (strcmp(block->header.merkle_root, compute_merkle_root(block->transactions)) != 0) {
        printf("❌ Error: Merkle root mismatch!\n");
        printf("Expected: %s\n", block->header.merkle_root);
        printf("Computed: %s\n", compute_merkle_root(block->transactions));
        return false;
    }
    return true;
}


Block* Block__Create(Block *prev_block, int difficulty, vector(Transaction) *transactions, const char *miner_pubkey) {
    Block *new_block = malloc(sizeof(Block));
    new_block->header = (BlockHeader){
        .index = prev_block ? prev_block->header.index + 1 : 0,
        .timestamp = time(NULL),
        .difficulty = difficulty,
        .nonce = malloc(sizeof(int)),
        .hash = malloc(65),
        .next = NULL,
    };

    *(new_block->header.nonce) = 0;
    memset(new_block->header.hash, 0, 65);
    new_block->transactions = transactions;
    if (prev_block) {
        strncpy(new_block->header.previous_hash, prev_block->header.hash, 65);
    } else {
        strcpy(new_block->header.previous_hash, "0");
    }


    Block__Mine(new_block, (char *)miner_pubkey);

    return new_block;
}


void Block__Append(Blockchain *chain, int difficulty, vector(Transaction) *transactions, const char *miner_pubkey) {
    Block *new_block;
    if (chain->head == NULL) {
        new_block = Block__Create(NULL, difficulty, transactions, miner_pubkey);

        if (!Block__Verify(chain, new_block, NULL)) {
            printf("❌ Error: New block is invalid!\n");
            return;
        }

        chain->head = new_block;
    } else {
        Block *last = chain->head;
        while (last->header.next != NULL)
            last = last->header.next;
        new_block = Block__Create(last, difficulty, transactions, miner_pubkey);


        if (!Block__Verify(chain, new_block, last)) {
            printf("❌ Error: New block is invalid!\n");
            printf("header 2 %p\n", &new_block->header);

            return;
        }

        last->header.next = new_block;
    }
}


void Block__Print(Block *current) {
    printf("🧾------------\n");
    printf("Index: %d\n", current->header.index);
    printf("Timestamp: %ld\n", current->header.timestamp);
    printf("PrevHash: %s\n", current->header.previous_hash);
    printf("Hash: %s\n", current->header.hash);
    printf("Nonce: %d\n", *(current->header.nonce));
    printf("Merkle Root: %s\n", current->header.merkle_root);
    printf("Transactions:\n");
    for (int i = 0; i < current->transactions->get_size(current->transactions); i++) {
        Transaction *transaction = current->transactions->at(current->transactions, i);
        Transaction__Print(transaction);
    }
    current = current->header.next;
}