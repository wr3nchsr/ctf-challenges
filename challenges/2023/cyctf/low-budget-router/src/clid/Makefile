CC = mips-linux-gnu-gcc
# CC = gcc
CFLAGS = -Wall -fPIC -Wimplicit-function-declaration -z noexecstack -w
LDFLAGS = -shared
SRC_DIR = src

# Source files for cli_functions and cli_helpers libraries
CLI_FUNCTIONS_SRC = $(SRC_DIR)/cli_functions.c
CLI_HELPERS_SRC = $(SRC_DIR)/cli_helpers.c

# Object files for cli_functions and cli_helpers libraries
CLI_FUNCTIONS_OBJ = $(patsubst $(SRC_DIR)/%.c,%.o,$(CLI_FUNCTIONS_SRC))
CLI_HELPERS_OBJ = $(patsubst $(SRC_DIR)/%.c,%.o,$(CLI_HELPERS_SRC))

# Main program source and object file
MAIN_SRC = $(SRC_DIR)/main.c
MAIN_OBJ = main.o

# Output shared library files
CLI_FUNCTIONS_SO = libcli_functions.so
CLI_HELPERS_SO = libcli_helpers.so

# Output main program executable
MAIN_EXE = clid.out

all: $(MAIN_EXE)

$(MAIN_EXE): $(MAIN_OBJ) $(CLI_FUNCTIONS_SO) $(CLI_HELPERS_SO)
	$(CC) -o $@ $< -L. -lcli_functions -lcli_helpers -pthread -z noexecstack 

$(CLI_FUNCTIONS_SO): $(CLI_FUNCTIONS_OBJ) $(CLI_HELPERS_SO)
	$(CC) $(LDFLAGS) -o $@ $< -L . -lcli_helpers

$(CLI_HELPERS_SO): $(CLI_HELPERS_OBJ)
	$(CC) $(LDFLAGS) -o $@ $<

%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(MAIN_EXE) $(CLI_FUNCTIONS_SO) $(CLI_HELPERS_SO) $(MAIN_OBJ) $(CLI_FUNCTIONS_OBJ) $(CLI_HELPERS_OBJ)

.PHONY: all clean
