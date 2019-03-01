include env.mk


MODULE_NAME = yconfig


ifeq ($(prefix),)
    BIN_DIR = $(shell pwd)
else
    BIN_DIR = $(prefix)/lib
endif

ifeq ($(build),)
    BUILD_DIR = $(shell pwd)/build
else
    BUILD_DIR = $(build)/$(MODULE_NAME)
endif

vpath %.o $(BUILD_DIR)


OBJS = yconfig.o

EXE = $(BIN_DIR)/test
LIB = $(BIN_DIR)/lib$(MODULE_NAME).a

.PHONY: all
all: $(LIB)

$(LIB): $(OBJS)
	@$(MKDIR) $(BIN_DIR)
	@cd $(BUILD_DIR)  &&  $(AR) rcs $(LIB) $(OBJS)  && echo -e "  AR	$(LIB) Success."

%.o: %.cpp
	@$(MKDIR) $(BUILD_DIR)
	@echo -e "  CXX		$<"  &&  $(CXX) -c $(CXXFLAGS) $< -o $(BUILD_DIR)/$@

%.d: %.cpp
	@set -e; $(RM) $@; $(CXX) -MM $< $(CXXFLAGS) > $@.$$$$; \
	    sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	    $(RM) $@.$$$$

-include $(OBJS:.o=.d)


.PHONY: clean
clean:
	@echo -e "  Clean		$(MODULE_NAME)"  &&  $(RM) $(EXE) $(LIB) $(BUILD_DIR) *.d *.d.*

test: test.o $(LIB)
	@$(MKDIR) $(BIN_DIR)
	@echo -e "  LD  $(EXE)  <<== [ test.o $(LIBS) ]"\
	    &&  cd $(BUILD_DIR)  &&  $(CXX) -o $(EXE) test.o $(LIB)
