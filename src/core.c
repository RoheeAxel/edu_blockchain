#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/rsa.h>

#include "Blockchain.h"
#include "vector.h"
#include "Block.h"
#include "Transaction/Transaction.h"

// Converts public key to PEM string (safely within 256 bytes)
void extract_pubkey_string(EVP_PKEY *key, char *out, size_t max_len) {
    BIO *bio = BIO_new(BIO_s_mem());
    PEM_write_bio_PUBKEY(bio, key);

    char *data;
    long len = BIO_get_mem_data(bio, &data);
    if (len >= max_len) len = max_len - 1;  // ensure null-termination
    strncpy(out, data, len);
    out[len] = '\0';

    BIO_free(bio);
}

EVP_PKEY *generate_keypair() {
    EVP_PKEY *pkey = EVP_RSA_gen(2048); // 2048-bit RSA
    if (!pkey) {
        fprintf(stderr, "Key generation failed\n");
        exit(1);
    }
    return pkey;
}

// void verify_transactions(vector(Transaction) *transactions, EVP_PKEY *pkey) { CHANGE Transaction__Verify to Transaction__checkUser or smth
//     printf("Verifying transactions...\n");
//     for (int i = 0; i < transactions->Size; i++) {
//         Transaction *tx = transactions->at(transactions, i);
//         if (Transaction__Verify(tx, pkey)) {
//             printf("Transaction %d is done by user\n", i);
//         } else {
//             printf("Transaction %d is invalid\n", i);
//         }
//     }
// }


vector(Transaction) *simulate_transactions(EVP_PKEY *pkey_1, EVP_PKEY *pkey_2, EVP_PKEY *pkey_3, char *_prev_txid) {
    srand(42);

    char pubkey_1[256];
    char pubkey_2[256];
    char pubkey_3[256];


    extract_pubkey_string(pkey_1, pubkey_1, sizeof(pubkey_1));
    extract_pubkey_string(pkey_2, pubkey_2, sizeof(pubkey_2));
    extract_pubkey_string(pkey_3, pubkey_3, sizeof(pubkey_3));

    char *pubkeys[] = {pubkey_1, pubkey_2, pubkey_3};
    EVP_PKEY *pkeys[] = {pkey_1, pkey_2, pkey_3};

    vector(Transaction) *transactions = newT(vector, Transaction, 0);


    int num_transactions = 1; //

    char prev_txid[65];
    if (_prev_txid) {
        strncpy(prev_txid, _prev_txid, sizeof(prev_txid));
    } else {
        memset(prev_txid, 0, sizeof(prev_txid));
    }

    for (int i = 0; i < num_transactions; i++) {
        vector(TxInput) *input_1 = newT(vector, TxInput, 0);
        vector(TxOutput) *output_1 = newT(vector, TxOutput, 0);

        input_1->push_back(input_1, TxInput__Create(prev_txid, 0));
        output_1->push_back(output_1, TxOutput__Create(pubkey_1, BLOCKCHAIN__COINBASE_REWARD));
        int sender_idx = rand() % 3;
        int receiver_idx;
        do {
            receiver_idx = rand() % 3;
        } while (receiver_idx == sender_idx);

        int amount = rand() % 100 + 1;

        transactions->push_back(transactions, Transaction__Create(input_1, output_1));
        Transaction__Sign(transactions->at(transactions, i), pkeys[sender_idx]);
        Transaction__Hash(transactions->at(transactions, i), prev_txid);
    }

    printf("Generated %d random transactions successfully!\n", num_transactions);
    return transactions;
}
int main() {
    OpenSSL_add_all_algorithms();

    EVP_PKEY *pkey_1 = generate_keypair(); // "Alice"
    EVP_PKEY *pkey_2 = generate_keypair(); // "Bob"
    EVP_PKEY *pkey_3 = generate_keypair(); // "Charlie"

    char pubkey_1[65];
    extract_pubkey_string(pkey_1, pubkey_1, sizeof(pubkey_1));

    Blockchain *blockchain = Blockchain__Create(4);

    int n_block = 3;


    vector(Transaction) **all_transactions = malloc(sizeof(vector(Transaction) *) * n_block);
    all_transactions[0] = newT(vector, Transaction, 0);
    Blockchain__Append(blockchain, all_transactions[0], pubkey_1); // Create the first block with a coinbase transaction

    char prev_txid[65];
    strncpy(prev_txid, all_transactions[0]->at(all_transactions[0], 0)->txid, sizeof(prev_txid));

    for (int i = 1; i < n_block; i++) {
        all_transactions[i] = simulate_transactions(pkey_1, pkey_2, pkey_3, prev_txid); // TODO check double spending
    }

    for (int i = 1; i < n_block; i++) {
        Blockchain__Append(blockchain, all_transactions[i], pubkey_1);
    }

    // Tryna cheat test
    // blockchain->head->transactions->at(blockchain->head->transactions, 0)->outputs->at(blockchain->head->transactions->at(blockchain->head->transactions, 0)->outputs, 0)->amount = 1000;


    printf("\n🧾 Blockchain:\n");
    Blockchain__Print(blockchain);


    printf("Balance of %s: %d\n", pubkey_1, Blockchain__GetBalance(blockchain, pubkey_1));


    for (int i = 0; i < n_block; i++) {
        deleteT(vector, Transaction, all_transactions[i]);
    }


    EVP_PKEY_free(pkey_1);
    EVP_PKEY_free(pkey_2);
    EVP_PKEY_free(pkey_3);

    return 0;
}
