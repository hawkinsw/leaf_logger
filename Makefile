HOST_GCC=g++
TARGET_GCC=gcc
PLUGIN_SOURCE_FILES=leaf_logger.cc
PLUGIN_OUTPUT_FILE=leaf_logger.so
GCCPLUGINS_DIR:= $(shell $(TARGET_GCC) -print-file-name=plugin)
CXXFLAGS+= -I$(GCCPLUGINS_DIR)/include -fPIC -fno-rtti

$(PLUGIN_OUTPUT_FILE): $(PLUGIN_SOURCE_FILES)
	$(HOST_GCC) -O0 -g -shared $(CXXFLAGS) $^ -o $@

test: $(PLUGIN_OUTPUT_FILE)
	gcc -fplugin=./$(PLUGIN_OUTPUT_FILE) test.c -fplugin-arg-leaf_logger-output="test.out" -g -O0
	cat test.out
	rm -rf a.out
	rm -f test.out

test-clean:
	rm -f a.out test.out *.leafs
clean: test-clean
	rm -f $(PLUGIN_OUTPUT_FILE)
