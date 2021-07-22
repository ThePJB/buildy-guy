APPNAME = buildy-guy
CC = g++
ODIR = crap
HERE = $(PWD)

# Test stuff
TEST_DIR = tests
TEST_SRCS = $(wildcard $(TEST_DIR)/*.cpp)
TEST_EXECUTABLES = $(patsubst $(TEST_DIR)/%.cpp, $(ODIR)/$(TEST_DIR)/%.out, ${TEST_SRCS})
TEST_OBJS = $(patsubst $(TEST_DIR)/%.cpp $(ODIR)/$(TEST_DIR)/%.o)

CFLAGS := \
	-O2 -g -Wall -Werror -Wfatal-errors \
	-march=native -mtune=native \
	-MMD # make dependencies

# Warning exceptions
CFLAGS += -Wno-unused-variable
CFLAGS += -Wno-unused-const-variable
CFLAGS += -Wno-unused-but-set-variable # might be a useful actually
CFLAGS += -Wno-narrowing # might be useful too, we will c
CFLAGS += -Wno-sign-compare
CFLAGS += -Wno-class-memaccess
CFLAGS += -Wno-strict-aliasing

CFLAGS += -I$(HERE)/danklib

#LDFLAGS = -lunwind
LDFLAGS = -lbacktrace -lSDL2 -lSDL2_gfx
SRCS = $(wildcard *.cpp)
SRCS += $(wildcard danklib/*.cpp)
OBJS := $(patsubst %.cpp, $(ODIR)/%.o, ${SRCS})
OBJS_NONMAIN := $(filter-out crap/main.o, ${OBJS})

.PHONY: all clean

all: dirs $(APPNAME)

dirs: 
	mkdir -p $(ODIR) $(TEST_DIR) $(ODIR)/$(TEST_DIR)

# Object Files
$(ODIR)/%.o: %.cpp
	$(CC) -o $@ -c $< $(CFLAGS)

# Application
$(APPNAME): $(OBJS)
	$(CC) -o  $(APPNAME) $^ $(LDFLAGS)

-include $(OBJS:.o=.d)

run: all
	./$(APPNAME)

gdb: all
	gdb -q -ex run ./$(APPNAME)

# Test obj files (can you have a heterogenous mix of src files and O files?)
$(ODIR)/$(TEST_DIR)/%.o: $(TEST_DIR)/%.cpp 
	$(CC) -o $@ -c $< $(CFLAGS)

# Test executables
$(ODIR)/$(TEST_DIR)/%.out: $(ODIR)/$(TEST_DIR)/%.o $(OBJS_NONMAIN)
	$(CC) -o $@ $^ $(LDFLAGS)

test: $(TEST_EXECUTABLES)
	for test_executable in $^ ; do \
		echo "------$$test_executable------" ; \
		./$$test_executable ; \
	done

clean:
	rm -f $(APPNAME) $(ODIR)/*.o $(ODIR)/*.d