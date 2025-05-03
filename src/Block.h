#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/sha.h>
#include "vector.h"
#include "Transaction/Transaction.h"

#pragma once

init_vector(Transaction);

#define MAX_TRANSACTIONS_PER_BLOCK 10

typedef struct BlockHeader {
    int index;
    time_t timestamp;
    char previous_hash[65];
    char merkle_root[65];
    char *hash;
    int *nonce;
    int difficulty;
    struct Block* next;
} BlockHeader;


typedef struct Block {
    BlockHeader header;
    vector(Transaction) *transactions;
} Block;

typedef struct Blockchain Blockchain;

void Block__Hash(Block *block, char *output);
void Block__Mine(Block *block, char *miner_pubkey);
bool Block__MeetDifficulty(Block *block);
bool Block__VerifyCoinbaseTransaction(Block *block);
bool Block__Verify(Blockchain *chain, Block *block, Block *prev_block);
Block* Block__Create(Block *prev_block, int difficulty, vector(Transaction) *transactions, const char *miner_pubkey);
void Block__Append(Blockchain *chain, int difficulty, vector(Transaction) *transactions, const char *miner_pubkey);
void Block__Print(Block *block);