CXX := g++
CXXFLAGS := -g -O0 -Wall -Wextra -MD
LDFLAGS := -L.

default: cquote

-include curlpp.mk

# rapidjson
CXXFLAGS += -Irapidjson/include

# termbox
-include termbox.mk

SRC:=\
	main.cpp

OBJ:=$(SRC:.cpp=.o)

cquote: libcurlpp.a
cquote: libtermbox.a
cquote: $(OBJ)
	$(CXX) $(CXXFLAGS) -o cquote $(OBJ) $(LDFLAGS) $(CURLPP_LIBS) -lcurlpp -ltermbox -pthread
clean: libcurlpp_clean libtermbox_clean
	-rm -f $(OBJ) $(OBJ:.o=.d)

-include $(OBJ:.o=.d)