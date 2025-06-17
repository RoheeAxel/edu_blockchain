CC = gcc
CFLAGS = -lcrypto -I./src
TARGET = amadeus
SRC = src/core.c src/Block.c src/Transaction/Transaction.c src/Transaction/UTXO.c src/MerkleTree.c src/Blockchain.c src/Transaction/UTXO_map.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

fclean:
	rm -f $(TARGET)
	rm -f src/*.o
	rm -f src/Transaction/*.o
	rm -f src/Transaction/UTXO/*.o
	rm -f src/Transaction/UTXO/UTXO.o
	rm -f src/Transaction/Transaction.o
	rm -f src/MerkleTree.o
	rm -f src/Blockchain.o
	rm -f src/UTXO_map.o

debug:
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) -g