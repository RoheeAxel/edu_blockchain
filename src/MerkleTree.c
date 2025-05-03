#include "MerkleTree.h"

char* compute_merkle_root(vector(Transaction) *transactions) {
    int count = transactions->get_size(transactions);
    if (count == 0) return strdup("0");

    char **hashes = malloc(sizeof(char *) * count);
    for (int i = 0; i < count; i++) {
        Transaction *tx = transactions->at(transactions, i);
        hashes[i] = malloc(65);
        Transaction__Hash(tx, hashes[i]);
    }

    while (count > 1) {
        int new_count = (count + 1) / 2;
        char **new_hashes = malloc(sizeof(char *) * new_count);

        for (int i = 0; i < count; i += 2) {
            char concat[129 + 1] = {0};
            strcpy(concat, hashes[i]);
            if (i + 1 < count)
                strcat(concat, hashes[i + 1]);
            else
                strcat(concat, hashes[i]);

            unsigned char hash[SHA256_DIGEST_LENGTH];
            SHA256((unsigned char *)concat, strlen(concat), hash);

            new_hashes[i / 2] = malloc(65);
            for (int j = 0; j < SHA256_DIGEST_LENGTH; j++)
                sprintf(new_hashes[i / 2] + j * 2, "%02x", hash[j]);
            new_hashes[i / 2][64] = '\0';
        }

        // Free old level
        for (int i = 0; i < count; i++)
            free(hashes[i]);
        free(hashes);

        hashes = new_hashes;
        count = new_count;
    }

    char *root = strdup(hashes[0]);
    free(hashes[0]);
    free(hashes);
    return root;
}
