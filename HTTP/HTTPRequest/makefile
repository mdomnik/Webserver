# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/10/30 13:11:26 by mdomnik           #+#    #+#              #
#    Updated: 2025/10/30 13:11:28 by mdomnik          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = test_http
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g

SRC_DIR = src
INC_DIR = inc
TEST_DIR = test
OBJ_DIR = obj

SRCS = \
	$(SRC_DIR)/HTTPRequest.cpp \
	$(SRC_DIR)/HTTPRequestUtils.cpp \
	$(TEST_DIR)/test_http_request.cpp

OBJS = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)
INCS = -I$(INC_DIR)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCS) $(OBJS) -o $(NAME)
	@echo "✅ Built: $(NAME)"

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
