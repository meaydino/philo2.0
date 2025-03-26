NAME = philo
CC = cc
CFLAGS = -Wall -Wextra -Werror
RM = rm -rf
OBJ = $(SRC:.c=.o)
SRC = forks_ctrl.c \
	  initialization.c \
	  main.c \
	  philo_dead_ctrl.c \
	  philosopher.c \
	  security_func.c \
	  utils.c
OBJ_DIR = obj
OBJ = $(addprefix $(OBJ_DIR)/, $(SRC:.c=.o))

# Color codes
GREEN = \033[0;32m
RED = \033[0;31m
YELLOW = \033[0;33m
RESET = \033[0m

all: $(NAME)

$(NAME): $(OBJ)
	@$(CC) $(CFLAGS) -o $(NAME) $(OBJ) && echo "$(GREEN)✅ Compilation successful!$(RESET)" || echo "$(RED)❌ Compilation failed!$(RESET)"

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(OBJ_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@$(RM) $(OBJ)
	@$(RM) $(OBJ_DIR)
	@echo "$(YELLOW)🧹 Cleaned successfully!$(RESET)"

fclean: clean
	@$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
