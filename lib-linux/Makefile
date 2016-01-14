ifdef HOST
	PLAT:=$(word 1, $(subst -, , $(HOST)))
	TARGET:=liblinux_tool_$(PLAT).a
	DYNAMIC_TARGET=liblinux_tool_$(PLAT).so
	OBJDIR:=obj_$(PLAT)
else
	TARGET:=liblinux_tool.a
	DYNAMIC_TARGET=liblinux_tool.so
	OBJDIR:=obj
endif

SRC_DIR=src src/SimpleNet

CPPFLAGS+=-Wall -g -D_DEBUG
#CPPFLAGS+=-Wall -O2
CPPFLAGS+=-Isrc -Isrc/SimpleNet
CPPFILES:=$(shell find $(SRC_DIR)  -maxdepth 1 -name "*.cpp")
CPPOBJS:=$(addprefix $(OBJDIR)/, $(CPPFILES:%.cpp=%.o))
CPPOBJDIRS:=$(addprefix $(OBJDIR)/, $(SRC_DIR))
LIBS+=
SO_FLAGS=


ZIP_NAME:=$(basename $(TARGET))
ZIP_FILES:=$(addsuffix /*.[hc]*, $(SRC_DIR) test) Makefile README.md

TESTFILES:=$(shell find test -maxdepth 1 -name "*.cpp")
TESTOBJS:=$(TESTFILES:%.cpp=%.o)
TESTAPP:=$(TESTOBJS:%.o=%)
CPPOBJDIRS+= $(OBJDIR)/test

all:$(CPPOBJDIRS) $(TARGET)

so: SO_FLAGS=-fPIC
so:$(CPPOBJDIRS) $(DYNAMIC_TARGET) 

example:$(CPPOBJDIRS) $(TARGET) $(TESTAPP)

-include $(addsuffix /*.d, $(CPPOBJDIRS))

$(TARGET):$(CPPOBJS)
	$(HOST)ar cru $(TARGET) $^

$(DYNAMIC_TARGET):$(CPPOBJS)
	$(HOST)g++ -shared -fPIC -o $@ $^
	$(HOST)strip $@

$(OBJDIR)/%.o:%.cpp
	$(HOST)g++ -c $(SO_FLAGS) $(CPPFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -o $@ $<

$(TESTAPP):%:$(OBJDIR)/%.o 
	$(HOST)g++ -o $@ $< $(TARGET) -lpthread -ldl

$(TESTOBJS)/%.o:%.cpp
	$(HOST)g++ -c $(CPPFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -o $@ $<

$(CPPOBJDIRS):
	@mkdir -p $@

zip: 
	-rm -rf $(ZIP_NAME).zip
	zip -r $(ZIP_NAME).zip $(ZIP_FILES)

clean:
	-rm -r -f $(addsuffix /*.[do], $(SRC_DIR) test) $(TESTAPP) $(OBJDIR)

.PHONY: zip test all example so
