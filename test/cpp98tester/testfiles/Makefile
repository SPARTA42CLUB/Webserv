.SUFFIXES : .cpp .o

NAME	:= a.out
CXX		:= c++
CFLAGS	:= -std=c++98 -Werror -Wall -Wextra

SRC		:= test.cpp
OBJ		:= $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	@$(CXX) $(CFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp
	@$(CXX) $(CFLAGS) -c $< -o $@ $(INCLUDE)

clean:
	@rm -f $(OBJ)

fclean: clean
	@rm -f $(NAME)

re:
	@$(MAKE) fclean
	@$(MAKE) all

mem:
	@$(MAKE) fclean
	@$(MAKE) all CFLAGS="$(CFLAGS) $(MEMORY)"

lldb:
	@$(MAKE) fclean
	@$(MAKE) all CFLAGS="$(CFLAGS) $(DEBUG)"

.PHONY: all clean fclean re mem lldb
