#include <stdbool.h>
#include "Block.h"
#include "Transaction/Transaction.h"
#include "vector.h"

#pragma once
#include "Transaction/UTXO_map.h"


#define BLOCKCHAIN__TARGET_TIME 5 // seconds
#define BLOCKCHAIN__DIFFICULTY_ADJUST_INTERVAL 5
#define BLOCKCHAIN__COINBASE_REWARD 8

typedef struct Blockchain {
    Block *head;
    int size;
    int difficulty;
    UTXO_map utxo_map;
} Blockchain;

Blockchain *Blockchain__Create(int difficulty);
Block *Blockchain__Get_N_Prev(Block *head, int n);
bool Blockchain__Validate(Blockchain *chain);
int Blockchain__GetBalance(Blockchain *chain, const char *pub_key);
Transaction *Blockchain__FindTransactionByID(Blockchain *chain, const char *txid);
Block *Blockchain__FindBlockByTransactionId(Blockchain *chain, const char *txid);
void Blockchain__IncreaseDifficulty(Blockchain *blockchain, Block *current);
void Blockchain__Append(Blockchain *blockchain, vector(Transaction) * transactions, const char *miner_pubkey);
void Blockchain__AddToUTXO_map(Blockchain *blockchain, vector(Transaction) *transactions);

void Blockchain__Print(Blockchain *blockchain);
