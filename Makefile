SRC_PATH?=src
OBJ_DIR?=obj
BIN_DIR?=bin
BIN    ?=mtgrep

PRJ_SRC :=$(wildcard $(SRC_PATH)/*.c)
OBJ_PATH:=$(BIN_DIR)/$(OBJ_DIR)
PRJ_OBJ :=$(patsubst $(SRC_PATH)/%.c,$(OBJ_PATH)/%.o,$(PRJ_SRC))
OBJ     :=$(PRJ_OBJ)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c
	
	@if [ ! -d $(OBJ_PATH) ]; then \
		mkdir -p $(OBJ_PATH); \
	fi

	gcc -g -Wall -Wextra -c $< -o $@ 

bin: $(PRJ_OBJ)
	gcc $(OBJ) -o $(BIN_DIR)/$(BIN)

clean:
	$(RM) -rf $(BIN_DIR)
	
.PHONY: clean