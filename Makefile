# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2017/12/10 17:05:19 by pgritsen          #+#    #+#              #
#    Updated: 2018/08/26 16:26:23 by pgritsen         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME_1		=	chat_server
NAME_2		=	42chat

CC			=	gcc

CFLAGS		=	-Wextra -Werror -Wall -O2 -g3 -fno-unwind-tables				\
				-fno-asynchronous-unwind-tables -Wno-cast-function-type
BFLAGS		=	-s -ffunction-sections -fdata-sections	-fno-ident				\
				-fmerge-all-constants

HDRSDIR		=	./includes

HDRS		=	$(addprefix $(HDRSDIR)/, node.h server.h client.h handler.h)

SRCSDIR		=	./sources

SRCS_1		=	server.c data_exchange.c data_validation.c cmd_helpers.c		\
				cmd_interfaces.c system.c msg_helpers.c msg_helpers_2.c			\
				cmd_interfaces_2.c data_helpers.c
SRCS_2		=	client.c data_exchange.c data_validation.c render.c io.c		\
				update_data.c curses.c key_handler.c readline.c data_helpers.c

OBJDIR		=	./obj

DIRS		=	./obj

OBJ_1		=	$(addprefix $(OBJDIR)/, $(SRCS_1:.c=.o))
OBJ_2		=	$(addprefix $(OBJDIR)/, $(SRCS_2:.c=.o))

LIBFOLDER	=	./libraries

LIBSDEPS	=	$(addprefix $(LIBFOLDER)/, libft/libft.a)

INCLUDES	=	-I./includes
INCLUDES	+=	$(addprefix -I$(LIBFOLDER)/, libft)

LIBRARIES	=	$(addprefix -L$(LIBFOLDER)/, libft) -lft -levent -lm

uniq		=	$(if $1,$(firstword $1) $(call uniq,$(filter-out $(firstword $1),$1)))

all: lib $(NAME_1) $(NAME_2)

$(NAME_1): $(OBJ_1) $(HDRS) $(LIBSDEPS)
	@printf "\n\033[32m[Creating $(NAME_1)].......\033[0m"
	@$(CC) $(CFLAGS) $(BFLAGS) -o $(NAME_1) $(OBJ_1) $(LIBRARIES)
	@printf "\033[32m[DONE]\033[0m\n"

$(NAME_2): $(OBJ_2) $(HDRS) $(LIBSDEPS)
	@printf "\n\033[32m[Creating $(NAME_2)].......\033[0m"
	@$(CC) $(CFLAGS) $(BFLAGS) -o $(NAME_2) $(OBJ_2) $(LIBRARIES) -lreadline -lncursesw
	@printf "\033[32m[DONE]\033[0m\n"

$(call uniq, $(OBJ_1) $(OBJ_2)): $(OBJDIR)/%.o : $(SRCSDIR)/%.c $(HDRS)
	@mkdir -p $(DIRS)
	@printf "\033[32m[Compiling $<].......\033[0m"
	@$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDES)
	@printf "\033[32m[DONE]\033[0m\n"

$(LIBSDEPS): lib

lib:
	@printf "\033[33m[Creating libft.a].......[START]\033[0m\n"
	@make -j3 -C $(LIBFOLDER)/libft
	@printf "\033[33m[Creating libft.a].......[END]\033[0m\n"

clean:
	@printf "\n\033[31m[Cleaning $(NAME_1) and $(NAME_2) object files].......\033[0m"
	@rm -rf $(OBJDIR)
	@printf "\033[31m[DONE]\033[0m\n"

clean_%:
	@make -j3 -C $(LIBFOLDER)/$(subst clean_,,$@) clean

fclean: clean
	@rm -rf $(NAME_1)
	@printf "\033[31m[Removing $(NAME_1)].......[DONE]\033[0m\n"
	@rm -rf $(NAME_2)
	@printf "\033[31m[Removing $(NAME_2)].......[DONE]\033[0m\n"

fclean_%:
	@make -j3 -C $(LIBFOLDER)/$(subst fclean_,,$@) fclean

fclean_all:
	@make -j3 -C $(LIBFOLDER)/libft fclean
	@make fclean

rebuild_message:
	@printf "\033[33m[Rebuild $(NAME_1) and $(NAME_2)].......[START]\033[0m\n"

re:	rebuild_message fclean all
	@printf "\033[33m[Rebuild $(NAME_1) and $(NAME_2)].......[END]\033[0m\n"

re_all:	rebuild_message
	@make -j3 -C $(LIBFOLDER)/libft re
	@make fclean
	@make all
	@printf "\033[33m[Rebuild $(NAME_1) and $(NAME_2)].......[END]\033[0m\n"

re_%:
	@make -j3 -C $(LIBFOLDER)/$(subst re_,,$@) re

.NOTPARALLEL: all $(NAME_1) $(NAME_2) re
