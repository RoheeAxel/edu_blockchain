
#include "vector.h"
#include "Block.h"
#include "Transaction/Transaction.h"

#pragma once
char* compute_merkle_root(vector(Transaction) *transactions);