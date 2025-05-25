SRC_PATH=../src
PRJ_SRC+=$(wildcard $(SRC_PATH)/*.c)

TEST_SRC=test.c
TEST_OBJ=$(BUILD)/test.o

PRJ_OBJ=$(patsubst $(SRC_PATH)/%.c,$(BUILD)/%.o,$(PRJ_SRC))
OBJ+=$(PRJ_OBJ)
OBJ+=$(TEST_OBJ)
BUILD=build

BIN=test

TEST_DATA_PATH=test_data/
TEST_PATTERN=TEST_PATTERN

# Build src
$(BUILD)/%.o: $(SRC_PATH)/%.c
	
	@if [ ! -d $(BUILD) ]; then \
		mkdir -p $(BUILD); \
	fi

	gcc -g -Wall -Wextra -c $< -o $@ 

# Build main

$(BUILD)/%.o: %.c
	
	if [ ! -d $(BUILD) ]; then \
		mkdir -p $(BUILD); \
	fi

	gcc -g -Wall -Wextra -c $< -o $@ 

build: $(PRJ_OBJ) $(TEST_OBJ)
	gcc $(OBJ) -o $(BUILD)/$(BIN)

test: build
	./gen_test.sh $(BUILD)/$(TEST_DATA_PATH) && (cd $(BUILD) && ./$(BIN) $(TEST_PATTERN) $(TEST_DATA_PATH))

clean:
	@$(RM) -rf $(BUILD)

.PHONY: build test clean
