SRC_DIR=src
#CFLAGS+=-Wall -g -D_DEBUG
#CFLAGS+=-Wall -g -DNDEBUG
CFLAGS+=-Wall
CFLAGS+=-Isrc 
CFLAGS+=-Ilibs/rpclib/include

LDFLAGS+=-Llibs/rpclib/lib -Llibs/curllib/lib
CFILES=$(shell find $(SRC_DIR)  -maxdepth 1 -name "*.c")
COBJS=$(CFILES:%.c=%.o)
LIBS+= -lxmlrpc_client -lxmlrpc -lxmlrpc_util\
	   -lxmlrpc_xmlparse -lxmlrpc_xmltok -lcurl
TARGET=libxmlrpc-hukong-client.a
TEST=client
ZIP_NAME:=libxmlrpc-hukong-client
ZIP_FILES:=$(addsuffix /*.[hc]*, $(SRC_DIR)) Makefile

all:$(TARGET) test

test:$(TEST)

-include $(addsuffix /*.d, $(SRC_DIR))

$(TARGET):$(COBJS)
	$(HOST)ar cru $(TARGET) $^

$(TEST):test/$(TEST).o $(TARGET) 
	$(HOST)gcc $(LDFLAGS) $^ $(LIBS) -o $@

$(COBJS) test/$(TEST).o:%.o:%.c
	$(HOST)gcc -c $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -o $@ $<

zip: 
	-rm -rf $(ZIP_NAME).zip
	zip -r $(ZIP_NAME).zip $(ZIP_FILES)

clean:
	-rm -f $(addsuffix /*.d, $(SRC_DIR) test) $(addsuffix /*.o, $(SRC_DIR) test) $(TARGET) $(TEST)

.PHONY:all clean zip test 
