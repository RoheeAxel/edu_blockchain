#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include "UTXO.h"
#include "Transaction.h"

TxInput *TxInput__Create(const char *prev_txid, int output_index)
{
    TxInput *input = malloc(sizeof(TxInput));
    if (!input) return NULL;

    strncpy(input->prev_txid, prev_txid, sizeof(input->prev_txid) - 1);
    input->prev_txid[sizeof(input->prev_txid) - 1] = '\0'; // Ensure null-termination
    input->output_index = output_index;
    memset(input->signature, 0, sizeof(input->signature)); // Initialize signature to zero

    return input;
}


size_t TxInput__Serialize(TxInput *input, char *output)
{
    size_t offset = 0;
    memcpy(output + offset, input->prev_txid, sizeof(input->prev_txid));
    offset += sizeof(input->prev_txid);
    memcpy(output + offset, &input->output_index, sizeof(int));
    offset += sizeof(int);
    memcpy(output + offset, input->signature, sizeof(input->signature));
    offset += sizeof(input->signature);
    return offset;
}


void TxInput__Print(TxInput *input)
{
    if (!input) return;
    printf("TxInput:\n");
    printf("  Previous Transaction ID: %s\n", input->prev_txid);
    printf("  Output Index: %d\n", input->output_index);
    printf("  Signature: ");
    for (int i = 0; i < 256; ++i)
        printf("%02x", (unsigned char)input->signature[i]);
    printf("\n");
}


void TxInput__Sign(TxInput *input, EVP_PKEY *private_key)
{
    if (!input || !private_key) return;

    unsigned char data_to_sign[sizeof(input->prev_txid) + sizeof(int)];
    size_t offset = 0;

    memcpy(data_to_sign + offset, input->prev_txid, sizeof(input->prev_txid));
    offset += sizeof(input->prev_txid);

    memcpy(data_to_sign + offset, &input->output_index, sizeof(int));
    offset += sizeof(int);

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) return;

    if (EVP_DigestSignInit(ctx, NULL, EVP_sha256(), NULL, private_key) != 1) goto cleanup;
    if (EVP_DigestSignUpdate(ctx, data_to_sign, offset) != 1) goto cleanup;

    size_t sig_len = 0;
    if (EVP_DigestSignFinal(ctx, NULL, &sig_len) != 1) goto cleanup;

    if (EVP_DigestSignFinal(ctx, (unsigned char *)input->signature, &sig_len) != 1) goto cleanup;

cleanup:
    EVP_MD_CTX_free(ctx);
}

bool TxInput__Verify(const TxInput *input, EVP_PKEY *public_key)
{
    if (!input || !public_key) return false;

    unsigned char data_to_verify[sizeof(input->prev_txid) + sizeof(int)];
    size_t offset = 0;

    memcpy(data_to_verify + offset, input->prev_txid, sizeof(input->prev_txid));
    offset += sizeof(input->prev_txid);

    memcpy(data_to_verify + offset, &input->output_index, sizeof(int));
    offset += sizeof(int);

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) return false;

    bool valid = false;

    if (EVP_DigestVerifyInit(ctx, NULL, EVP_sha256(), NULL, public_key) != 1) goto cleanup;
    if (EVP_DigestVerifyUpdate(ctx, data_to_verify, offset) != 1) goto cleanup;

    // We assume the signature has a known max length, or you can pass `input->signature_len` if you store that
    if (EVP_DigestVerifyFinal(ctx, (const unsigned char *)input->signature, 256) == 1)
        valid = true;

cleanup:
    EVP_MD_CTX_free(ctx);
    return valid;

}

TxOutput *TxOutput__Create(const char *recipient, int amount)
{
    TxOutput *output = malloc(sizeof(TxOutput));
    if (!output) return NULL;

    strncpy(output->recipient, recipient, sizeof(output->recipient) - 1);
    output->recipient[sizeof(output->recipient) - 1] = '\0'; // Ensure null-termination
    output->amount = amount;

    return output;
}

size_t TxOutput__Serialize(TxOutput *output, char *data)
{
    size_t offset = 0;
    memcpy(data + offset, output->recipient, sizeof(output->recipient));
    offset += sizeof(output->recipient);
    memcpy(data + offset, &output->amount, sizeof(int));
    offset += sizeof(int);
    return offset;
}

void TxOutput__Print(TxOutput *output)
{
    if (!output) return;
    printf("TxOutput:\n");
    printf("  Recipient: %s\n", output->recipient);
    printf("  Amount: %d\n", output->amount);
}
