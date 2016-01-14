HOST=mips-openwrt-linux-
SRC_DIR=src

CFLAGS+=-Wall -O2
#CFLAGS+=-Wall -g -D_DEBUG
CFILES=$(shell find $(SRC_DIR)  -maxdepth 1 -name "*.c")
COBJS=$(CFILES:%.c=%.o)
LIBS+= 
TARGET=ymodem_send
LDFLAGS= 

all:$(TARGET)

-include $(addsuffix /*.d, $(SRC_DIR))

$(TARGET):$(COBJS)
	$(HOST)gcc $(LDFLAGS) -o $@ $^ $(LIBS)

$(COBJS):%.o:%.c
	$(HOST)gcc -c $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -o $@ $<

clean:
	-rm -f $(addsuffix /*.d, $(SRC_DIR)) $(addsuffix /*.o, $(SRC_DIR)) $(TARGET)

.PHONY:all clean
