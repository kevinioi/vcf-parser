CC = gcc
CFLAGS = -Wall -g -std=c11

INCLUDE=-I./include
BIN=./bin/
SRC=./src/


all: parser list

writeHelper: $(SRC)WriteCardHelper.c ./include/WriteCardHelper.h
	$(CC) $(CFLAG) $(INCLUDE) -c $(SRC)WriteCardHelper.c -o $(BIN)writeCardHelper.o

parse: $(SRC)VCardParser.c ./include/VCardParser.h
	$(CC) $(CFLAG) $(INCLUDE) -c $(SRC)VCardParser.c -o $(BIN)parser.o

validate: $(SRC)ValidationHelper.c ./include/ValidationHelper.h
	$(CC) $(CFLAG) $(INCLUDE) -c $(SRC)ValidationHelper.c -o $(BIN)validate.o

parseHelper: $(SRC)ParseHelper.c ./include/VCardParser.h
	$(CC) $(CFLAG) $(INCLUDE) -c $(SRC)ParseHelper.c -o $(BIN)parseHelper.o

cardHelper: $(SRC)CardHelper.c ./include/CardHelper.h
	$(CC) $(CFLAG) $(INCLUDE) -c $(SRC)CardHelper.c -o $(BIN)cardHelper.o

propertyHelper: $(SRC)PropertyHelper.c ./include/PropertyHelper.h
	$(CC) $(CFLAG) $(INCLUDE) -c $(SRC)PropertyHelper.c -o $(BIN)propertyHelper.o

listAPI: $(SRC)LinkedListAPI.c ./include/LinkedListAPI.h
	$(CC) $(CFLAG) $(INCLUDE) -c $(SRC)LinkedListAPI.c -o $(BIN)listAPI.o

dateTime: $(SRC)DateHelper.c ./include/DateHelper.h
	$(CC) $(CFLAG) $(INCLUDE) -c $(SRC)DateHelper.c -o $(BIN)DateHelper.o

test:
	$(CC) $(CFLAG) $(INCLUDE) -c $(SRC)tester.c -o $(BIN)test.o

parser: parseHelper parse listAPI cardHelper propertyHelper dateTime validate
	ar cr $(BIN)libcparse.a $(BIN)DateHelper.o $(BIN)listAPI.o $(BIN)propertyHelper.o $(BIN)cardHelper.o $(BIN)parseHelper.o $(BIN)parser.o $(BIN)validate.o

list: listAPI
	ar cr $(BIN)libllist.a $(BIN)listAPI.o

clear:
	-rm -f bin/*.o
	-rm -f bin/*.a
