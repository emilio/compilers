TARGETS := target/TokenizerTest # parser evaluator
SOURCES := $(wildcard *.cc)
TARGET_OBJS := $(patsubst %.cc, target/%.o, $(SOURCES))
CFLAGS := -Wall -Werror -pedantic

.PHONY: all
all: $(TARGETS)
	@echo > /dev/null

.PHONY: fomat
format: $(SOURCES)
	for f in $(SOURCES); do clang-format -i $$f; done

target:
	mkdir -p $@

target/%: target/%.o $(TARGET_OBJS)
	$(CXX) $(CFLAGS) -o $@ $^

target/%.o: %.cc %.h target
	$(CXX) $(CFLAGS) -c $< -o $@

target/%.o: %.cc target
	$(CXX) $(CFLAGS) -c $< -o $@
