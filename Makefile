OPENWRT_DIR	:= /home/au/all/openwrt/attitude_adjustment
STAGING_DIR	:= $(OPENWRT_DIR)/staging_dir
CROSS_DIR	:= $(STAGING_DIR)/toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33.2/bin
export PATH	:= $(PATH):$(CROSS_DIR)
export STAGING_DIR

HOST:=mips-openwrt-linux-
#XMLRPC-C-CONFIG-PREFIX=/home/bobo/work/485/xmlrpc-openwrt-ar71xx/bin/
XMLRPC-C-CONFIG-PREFIX=./xmlrpc-openwrt-ar71xx/bin/

SRC_DIR:=src src/protocol
CPPFLAGS+=-Wall
#CPPFLAGS+=-Wall -g -D_DEBUG
#CPPFLAGS+=-Wall -g -D_DEBUG -D_TEST
#CPPFLAGS+=-Wall -O2 -DNDEBUG
CPPFLAGS+=-Isrc -Ilib-linux/src -Isrc/protocol -I./xmlrpc-openwrt-ar71xx/include
CPPFLAGS+=$(shell $(XMLRPC-C-CONFIG-PREFIX)xmlrpc-c-config c++2 abyss-server --cflags)
CPPFILES:=$(shell find $(SRC_DIR)  -maxdepth 1 -name "*.cpp")
CPPOBJS:=$(CPPFILES:%.cpp=%.o)
LIBS+= $(filter-out -lpthread, $(shell $(XMLRPC-C-CONFIG-PREFIX)xmlrpc-c-config c++2 abyss-server --libs))\
					lib-linux/liblinux_tool_mips.a -lpthread \
					-L./xmlrpc-openwrt-ar71xx/lib/ -lxmlrpc \
				   	-lxmlrpc_util \
					-lxmlrpc_xmlparse \
					-lxmlrpc_xmltok \
					-lxmlrpc_server \
					-lxmlrpc_abyss \
					-lxmlrpc_server_abyss 

TARGET:=server485
LDFLAGS:=-lm -ldl
ZIP_NAME:=Hukong
ZIP_FILES:=$(addsuffix /*.[hc]*, $(SRC_DIR) client/src client/test) client/Makefile Makefile

all:lib-linux $(TARGET)

-include $(addsuffix /*.d, $(SRC_DIR))

lib-linux:
	make HOST="$(HOST)" -C lib-linux

$(TARGET):$(CPPOBJS) lib-linux/liblinux_tool_mips.a
	$(HOST)g++ $(LDFLAGS) -o $@ $^ $(LIBS)
	$(HOST)strip $(TARGET)
	#sudo cp $(TARGET) openwrt/
	cp $(TARGET) openwrt/

$(CPPOBJS):%.o:%.cpp
	$(HOST)g++ -c $(CPPFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -o $@ $<

clean:
	-rm -f $(addsuffix /*.[do], $(SRC_DIR)) $(TARGET)

zip: 
	-rm -rf $(ZIP_NAME).zip
	zip -r $(ZIP_NAME).zip $(ZIP_FILES)

.PHONY:
	all clean zip lib-linux 
