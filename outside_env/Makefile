HOST:=mips-openwrt-linux-

ifdef HOST
	PLAT:=$(word 1, $(subst -, , $(HOST)))
	OBJDIR:=obj_$(PLAT)
	TARGET:=outside_env
	LIBS += lib-linux/liblinux_tool_$(word 1, $(subst -, , $(HOST))).a
else
	OBJDIR:=obj
	TARGET:=outside_env_x86
	LIBS += lib-linux/liblinux_tool.a
endif

SRC_DIR:=src src/protocol

CPPFLAGS+=-Wall -g -D_DEBUG
#CPPFLAGS+=-Wall -O2 -DNDEBUG
CPPFLAGS+=-Isrc -Isrc/protocol -Ilib-linux/src
CPPFILES:=$(shell find $(SRC_DIR)  -maxdepth 1 -name "*.cpp")
CPPOBJS:=$(addprefix $(OBJDIR)/, $(CPPFILES:%.cpp=%.o))
CPPOBJDIRS:=$(addprefix $(OBJDIR)/, $(SRC_DIR))
LIBS+=-lpthread
	   
LDFLAGS:=-lm -ldl
ZIP_NAME:=$(TARGET)
ZIP_FILES:=$(addsuffix /*.[hc]*, $(SRC_DIR)) Makefile

all:$(CPPOBJDIRS) $(TARGET)

-include $(addsuffix /*.d, $(CPPOBJDIRS))

$(TARGET):$(CPPOBJS)
	$(HOST)g++ $(LDFLAGS) -o $@ $^ $(LIBS)
	$(HOST)strip $(TARGET)
	sudo cp $(TARGET) openwrt/

$(OBJDIR)/%.o:%.cpp
	$(HOST)g++ -c $(CPPFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -o $@ $<

$(CPPOBJDIRS):
	@mkdir -p $@

clean:
	-rm -r -f $(addsuffix /*.[do], $(SRC_DIR)) $(TARGET) $(OBJDIR)

zip: 
	-rm -rf $(ZIP_NAME).zip
	zip -r $(ZIP_NAME).zip $(ZIP_FILES)

.PHONY:all clean zip
