CFLAGS := -g -O2 -Wall -Wextra -MD
CXXFLAGS += -Itermbox/src

libtermbox_src :=\
    termbox/src/termbox.c\
    termbox/src/utf8.c\

libtermbox_obj := $(libtermbox_src:.c=.o)
libtermbox.a: $(libtermbox_obj)
	ar -rcs libtermbox.a $(libtermbox_obj)

.PHONY:libcurlpp_clean
libtermbox_clean:
	-rm -f $(libtermbox_obj) libtermbox.a