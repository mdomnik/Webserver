# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/10/28 15:25:56 by mdomnik           #+#    #+#              #
#    Updated: 2025/10/28 15:25:58 by mdomnik          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# ================= Webserv Server Module ================= #

NAME     = test_server
CXX      = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g

SRC_DIR  = src
INC_DIR  = inc
TEST_DIR = test
OBJ_DIR  = obj

# ==== Source Files ==== #
SRCS = \
	$(SRC_DIR)/Server.cpp \
	$(TEST_DIR)/test_server.cpp \
	../Config/src/ConfigParser.cpp

OBJS = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)
INCS = -I$(INC_DIR) -I../Config/inc

# ================= Build Rules ================= #

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCS) $(OBJS) -o $(NAME)
	@echo "✅ Build complete: $(NAME)"

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
