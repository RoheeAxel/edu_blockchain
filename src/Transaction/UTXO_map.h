#pragma once
#include "std__map.h"
#include <string.h>
#include "vector.h"

init_vector(int);


typedef char txid_t[65];

init_std__map(txid_t, int_vector);

int txid_t__compare(txid_t *a, txid_t *b);
typedef std__map(txid_t, int_vector) *UTXO_map;