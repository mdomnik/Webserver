# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/10/31 15:47:42 by mdomnik           #+#    #+#              #
#    Updated: 2025/10/31 16:47:46 by mdomnik          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# ===== Compiler Settings =====
CXX      = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g

OBJ_DIR  = obj
TEST_DIR = tests
CFG_DIR  = ConfigFiles

# ===== Project Directories =====
SRV_DIR   = Server
CFG_SRC   = Config
HTTP_DIR  = HTTP
REQ_DIR   = $(HTTP_DIR)/HTTPRequest
RESP_DIR  = $(HTTP_DIR)/HTTPResponse
CGI_DIR   = $(HTTP_DIR)/CGI

# ===== Includes =====
INCS = -I$(SRV_DIR)/inc -I$(CFG_SRC)/inc -I$(REQ_DIR)/inc -I$(RESP_DIR)/inc -I$(CGI_DIR)/inc

# ===== Source Files =====
SRCS = \
	$(SRV_DIR)/src/Server.cpp \
	$(SRV_DIR)/src/ServerManager.cpp \
	$(CFG_SRC)/src/ConfigParser.cpp \
	$(CFG_SRC)/src/ServerConfig.cpp \
	$(REQ_DIR)/src/HTTPRequest.cpp \
	$(REQ_DIR)/src/HTTPRequestUtils.cpp \
	$(RESP_DIR)/src/HTTPResponse.cpp \
	$(RESP_DIR)/src/HTTPResponseMethods.cpp \
	$(RESP_DIR)/src/HTTPResponseUtils.cpp \
	$(CGI_DIR)/src/CGIHandler.cpp \

OBJS = $(addprefix $(OBJ_DIR)/, $(SRCS:.cpp=.o))

# ===== Tests =====
TESTS = \
	test_config \
	test_server \
	test_http_basic \
	test_http_error \
	test_cgi \
	test_integration \
	test_main

TEST_SRCS = $(addprefix $(TEST_DIR)/, $(addsuffix .cpp, $(TESTS)))
TEST_OBJS = $(addprefix $(OBJ_DIR)/, $(TEST_SRCS:.cpp=.o))

# ===== Build Rules =====
all: $(TESTS)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCS) -c $< -o $@

$(TESTS): %: $(OBJS) $(OBJ_DIR)/$(TEST_DIR)/%.o
	$(CXX) $(CXXFLAGS) $(INCS) $^ -o $@
	@echo "✅ Built $@"

# ===== Run Shortcuts =====
run_config: test_config
	./test_config $(CFG_DIR)/basic.conf

run_server: test_server
	./test_server $(CFG_DIR)/basic.conf

run_http: test_http_basic
	./test_http_basic $(CFG_DIR)/basic.conf

run_error: test_http_error
	./test_http_error $(CFG_DIR)/basic.conf

run_cgi: test_cgi
	./test_cgi $(CFG_DIR)/cgi.conf

run_integration: test_integration
	./test_integration $(CFG_DIR)/multi_server.conf

# ===== Cleaning =====
clean:
	rm -rf $(OBJ_DIR)
	@echo "🧹 Cleaned object files"

fclean: clean
	rm -f $(TESTS)
	@echo "🧼 Cleaned binaries"

re: fclean all

.PHONY: all clean fclean re run_config run_server run_http run_error run_cgi run_integration
