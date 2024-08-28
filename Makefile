CFLAGS=-std=c11 -g -static

TARGET = 9cc

SRCS = 9cc.c tokenizer.c utils.c parser.c codegen.c

OBJS = $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

9cc.o: 9cc.c tokenizer.h utils.h codegen.h parser.h
tokenizer.o: tokenizer.c tokenizer.h utils.h
utils.o: utils.c utils.h
parser.o: parser.c parser.h tokenizer.h utils.h
codegen.o: codegen.c codegen.h tokenizer.h utils.h parser.h

test: $(TARGET)
	./test.sh

clean:
	rm -f $(TARGET) *.o *~ tmp*

.PHONY: test clean
