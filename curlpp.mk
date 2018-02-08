CURLPP_FLAGS := $(shell pkg-config --cflags libcurl)
CURLPP_LIBS  := $(shell pkg-config --libs libcurl)

CXXFLAGS += -Icurlpp/include $(CURLPP_FLAGS)

libcurlpp_src :=\
    curlpp/src/curlpp/Easy.cpp\
    curlpp/src/curlpp/Exception.cpp\
    curlpp/src/curlpp/Form.cpp\
    curlpp/src/curlpp/Info.cpp\
    curlpp/src/curlpp/Multi.cpp\
    curlpp/src/curlpp/OptionBase.cpp\
    curlpp/src/curlpp/Options.cpp\
    curlpp/src/curlpp/cURLpp.cpp\
    curlpp/src/curlpp/internal/CurlHandle.cpp\
    curlpp/src/curlpp/internal/OptionList.cpp\
    curlpp/src/curlpp/internal/OptionSetter.cpp\
    curlpp/src/curlpp/internal/SList.cpp\

libcurlpp_obj := $(libcurlpp_src:.cpp=.o)
libcurlpp.a: $(libcurlpp_obj)
	ar -rcs libcurlpp.a $(libcurlpp_obj)

.PHONY:libcurlpp_clean
libcurlpp_clean:
	-rm -f $(libcurlpp_obj) libcurlpp.a