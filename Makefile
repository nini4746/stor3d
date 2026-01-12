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
			  $(SRC_DIR)/hdd/hdd_init.c \
			  $(SRC_DIR)/ssd/ssd_init.c

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

.PHONY: all clean fclean re
