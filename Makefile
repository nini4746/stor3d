# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/01/12 13:30:00 by yunhpark          #+#    #+#              #
#    Updated: 2026/01/12 13:30:00 by yunhpark         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		= stor3D

CC			= cc
CFLAGS		= -Wall -Wextra -Werror
INCLUDES	= -I./include

SRC_DIR		= src
OBJ_DIR		= obj

SRCS		= $(SRC_DIR)/main.c \
			  $(SRC_DIR)/init/validation.c \
			  $(SRC_DIR)/init/context.c \
			  $(SRC_DIR)/device/block_device.c \
			  $(SRC_DIR)/parser/parser.c \
			  $(SRC_DIR)/parser/commands.c \
			  $(SRC_DIR)/parser/file_commands.c \
			  $(SRC_DIR)/parser/utils.c \
			  $(SRC_DIR)/hdd/hdd_init.c \
			  $(SRC_DIR)/hdd/hdd_helpers.c \
			  $(SRC_DIR)/hdd/hdd_cache.c \
			  $(SRC_DIR)/hdd/hdd_io.c \
			  $(SRC_DIR)/ssd/ssd_init.c \
			  $(SRC_DIR)/ssd/ssd_ftl.c \
			  $(SRC_DIR)/ssd/ssd_gc.c \
			  $(SRC_DIR)/ssd/ssd_gc_helpers.c \
			  $(SRC_DIR)/ssd/ssd_io.c \
			  $(SRC_DIR)/filesystem/fs_init.c \
			  $(SRC_DIR)/filesystem/fs_alloc.c \
			  $(SRC_DIR)/filesystem/fs_file.c \
			  $(SRC_DIR)/filesystem/fs_ops.c

OBJS		= $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

# 단위 테스트: src/main.c 를 제외한 라이브러리 오브젝트만 링크
LIB_OBJS = $(filter-out $(OBJ_DIR)/main.o,$(OBJS))

tests/test_hdd_cache: tests/test_hdd_cache.c $(LIB_OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LIB_OBJS)

unit-test: tests/test_hdd_cache
	@./tests/test_hdd_cache

.PHONY: all clean fclean re unit-test
