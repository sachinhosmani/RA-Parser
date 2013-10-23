FILES=main.cpp table.cpp predicate.cpp tokenizer.cpp parser.cpp
CC=g++
CFLAGS=-w
EXE=a.out
compile : main.cpp table.cpp predicate.cpp tokenizer.cpp parser.cpp
	$(CC) $(FILES) $(CFLAGS) -o $(EXE)
