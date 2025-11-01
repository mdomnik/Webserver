# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/10/31 14:20:00 by mdomnik           #+#    #+#              #
#    Updated: 2025/10/31 17:43:22 by mdomnik          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

CXX      = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g

OBJ_DIR  = obj
NAME     = webserv

# --- Folders ---
SRV_DIR = Server
CFG_DIR = Config
HTTP_DIR = HTTP

# --- Subdirectories ---
SRV_INC = $(SRV_DIR)/inc
SRV_SRC = $(SRV_DIR)/src
CFG_INC = $(CFG_DIR)/inc
CFG_SRC = $(CFG_DIR)/src
REQ_INC = $(HTTP_DIR)/HTTPRequest/inc
REQ_SRC = $(HTTP_DIR)/HTTPRequest/src
RESP_INC = $(HTTP_DIR)/HTTPResponse/inc
RESP_SRC = $(HTTP_DIR)/HTTPResponse/src
CGI_INC = $(HTTP_DIR)/CGI/inc
CGI_SRC = $(HTTP_DIR)/CGI/src

INCS = -I$(SRV_INC) -I$(CFG_INC) -I$(REQ_INC) -I$(RESP_INC) -I$(CGI_INC)

# --- Source Files ---
SRCS = \
	main.cpp \
	$(SRV_SRC)/Server.cpp \
	$(SRV_SRC)/ServerManager.cpp \
	$(CFG_SRC)/ConfigParser.cpp \
	$(CFG_SRC)/ServerConfig.cpp \
	$(REQ_SRC)/HTTPRequest.cpp \
	$(REQ_SRC)/HTTPRequestUtils.cpp \
	$(RESP_SRC)/HTTPResponse.cpp \
	$(RESP_SRC)/HTTPResponseMethods.cpp \
	$(RESP_SRC)/HTTPResponseUtils.cpp \
	$(CGI_SRC)/CGIHandler.cpp

OBJS = $(addprefix $(OBJ_DIR)/, $(SRCS:.cpp=.o))

# --- Rules ---
all: $(NAME)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCS) -c $< -o $@

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCS) $(OBJS) -o $(NAME)
	@echo "✅ Built $(NAME)"

clean:
	rm -rf $(OBJ_DIR)
	@echo "🧹 Objects cleaned"

fclean: clean
	rm -f $(NAME)
	@echo "🧼 Binary removed"

re: fclean all

run:
	./$(NAME) ConfigFiles/default.conf

.PHONY: all clean fclean re run
