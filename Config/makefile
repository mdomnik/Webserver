# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/10/28 13:34:08 by mdomnik           #+#    #+#              #
#    Updated: 2025/10/28 14:02:12 by mdomnik          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# ================= Webserv Config Parser ================= #

NAME     = ../test_config
CXX      = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g

SRC_DIR  = src
INC_DIR  = inc
TEST_DIR = test
OBJ_DIR  = obj

SRCS = $(SRC_DIR)/ConfigParser.cpp \
       $(TEST_DIR)/test_config.cpp

OBJS = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)
INCS = -I$(INC_DIR)

# ================= Build Rules ================= #

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCS) $(OBJS) -o $(NAME)
	@echo "✅ Build complete."

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)
	@echo "🧹 Cleaned object files."

fclean: clean
	rm -f $(NAME)
	@echo "🧹 Removed binary."

re: fclean all

.PHONY: all clean fclean re
