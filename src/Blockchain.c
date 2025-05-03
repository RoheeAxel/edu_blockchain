#include "Blockchain.h"


Block *Blockchain__Get_N_Prev(Block *head, int n) {
    Block *current = head;
    int count = 0;

    // Count total number of blocks
    while (current) {
        count++;
        current = current->header.next;
    }

    if (count <= n) {
        return NULL;  // Not enough blocks yet
    }

    // Go to the (count - n - 1)th block from head (or count - n)
    int target = count - n;
    current = head;
    for (int i = 1; i < target; i++) {  // careful: start at 1 to land correctly
        current = current->header.next;
    }

    return current;
}


bool Blockchain__Validate(Blockchain *chain) {
    Block *current = chain->head;
    if (!(current, NULL)) {
        printf("Error: Blockchain is invalid!\n");
        return false;
    }
    while (current && current->header.next) {
        if (!Block__Verify(chain, current->header.next, current)) {
            printf("Error: Blockchain is invalid!\n");
            return false;
        }
        current = current->header.next;
    }
    return true;
}


Blockchain *Blockchain__Create(int difficulty)
{
    Blockchain *blockchain = malloc(sizeof(Blockchain));
    if (!blockchain) return NULL;

    blockchain->head = NULL;
    blockchain->difficulty = difficulty;

    return blockchain;
}


void Blockchain__IncreaseDifficulty(Blockchain *blockchain, Block *current)
{
    if (blockchain->size % BLOCKCHAIN__DIFFICULTY_ADJUST_INTERVAL == 0) {

        Block *n_prev = Blockchain__Get_N_Prev(blockchain->head, BLOCKCHAIN__DIFFICULTY_ADJUST_INTERVAL);
        if (!n_prev) {
            return;
        }

        time_t time_taken = current->header.timestamp - n_prev->header.timestamp;
        printf("Time taken: %ld seconds\n", time_taken);
        if (time_taken < BLOCKCHAIN__TARGET_TIME * BLOCKCHAIN__DIFFICULTY_ADJUST_INTERVAL) {
            printf("Increasing difficulty\n");
            blockchain->difficulty++;
        } else if (time_taken > BLOCKCHAIN__TARGET_TIME) {
            printf("Decreasing difficulty\n");
            blockchain->difficulty--;
        }
    }
}


Transaction *Blockchain__FindTransactionByID(Blockchain *chain, const char *txid) {
    Block *current = chain->head;
    while (current) {
        for (int i = 0; i < len(current->transactions); i++) {
            Transaction *tx = value_at(current->transactions, i);
            if (strcmp(tx->txid, txid) == 0) {
                return tx;
            }
        }
        current = current->header.next;
    }
    return NULL;  // Not found
}


Block *Blockchain__FindBlockByTransactionId(Blockchain *chain, const char *txid) {
    Block *current = chain->head;
    while (current) {
        for (int i = 0; i < len(current->transactions); i++) {
            Transaction *tx = value_at(current->transactions, i);
            if (strcmp(tx->txid, txid) == 0) {
                return current;
            }
        }
        current = current->header.next;
    }
    return NULL;  // Not found
}

int Blockchain__GetBalance(Blockchain *chain, const char *pub_key) {
    int balance = 0;

    Block *current = chain->head;
    while (current) {
        printf("Block %d:\n", current->header.index);
        if (!Block__Verify(chain, current, NULL)) {
            printf("Error: Blockchain is invalid!\n");
            return -1;
        }

        for (int i = 0; i < len(current->transactions); i++) {
            Transaction *tx = value_at(current->transactions, i);
            // Check outputs: add to balance if it belongs to pub_key
            for (int j = 0; j < len(tx->outputs); j++) {
                TxOutput *output = value_at(tx->outputs, j);
                printf("Recipient: %s\n", output->recipient);
                printf("Amount: %d\n", output->amount);
                if (strcmp(output->recipient, pub_key) == 0) {
                    balance += output->amount;
                }
            }

            // Check inputs: subtract if this pub_key is the sender
            for (int j = 0; j < len(tx->inputs); j++) {
                TxInput *input = value_at(tx->inputs, j);

                Transaction *prev_tx = Blockchain__FindTransactionByID(chain, input->prev_txid);
                if (prev_tx) {
                    TxOutput *spent_output = value_at(prev_tx->outputs, input->output_index);
                    if (strcmp(spent_output->recipient, pub_key) == 0) {
                        balance -= spent_output->amount;
                    }
                }
            }
        }
        current = current->header.next;
    }
    return balance;
}


void Blockchain__Append(Blockchain *blockchain, vector(Transaction) * transactions, const char *miner_pubkey)
{
    Block__Append(blockchain, blockchain->difficulty, transactions, miner_pubkey);

    if (!blockchain->head) {
        printf("Error: Failed to append block!\n");
        return;
    }
    Block *current = blockchain->head;
    blockchain->size = 0;
    while (current->header.next) {
        current = current->header.next;
        blockchain->size++;
    }
    blockchain->size++;  // Include the head block
}


void Blockchain__Print(Blockchain *blockchain)
{
    Block *current = blockchain->head;
    while (current) {
        Block__Print(current);
        current = current->header.next;
    }
    printf("%p\n", blockchain->head);
    printf("Difficulty: %d\n", blockchain->difficulty);
}