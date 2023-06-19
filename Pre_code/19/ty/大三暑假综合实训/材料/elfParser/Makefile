OBJS = main.o elfParser.o
TARGET = elfParser
CC = gcc
CFLAGS = --std=c99

$(TARGET): $(OBJS)  
	$(CC) $(OBJS) -o $@ 
	@echo "do gcc -o ..."

%.d: %.c
	@echo "create .d file ... now!"
	@set -e;rm -f $@;\
	$(CC) -M $(CFLAGS) $< >$@.$$$$;\
	sed 's,\($*\)\.o[ :]*,\1.o $@ :,g'<$@.$$$$>$@;\
	rm -f $@.$$$$

include $(OBJS:.o=.d)

.PHONY:install
install:
	cp $(TARGET) /usr/local/bin

.PHONY:clean
clean:
	rm -rf $(OBJS) $(TARGET) *.d 
