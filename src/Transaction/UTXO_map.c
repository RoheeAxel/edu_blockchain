#include "UTXO_map.h"

int txid_t__compare(txid_t *a, txid_t *b) {
    return strcmp(*a, *b);
}
