TARGETS := target/TokenizerTest target/EvaluatorTest # parser evaluator
SOURCES := $(wildcard *.cc)
TARGET_OBJS := $(patsubst %.cc, target/%.o, $(SOURCES))
CFLAGS := -Wall -Werror -pedantic -std=c++14 -g

.PHONY: all
all: $(TARGETS)
	@echo > /dev/null

.PHONY: format
format: $(SOURCES)
	for f in $(SOURCES); do clang-format -i $$f; done
	for f in $(wildcard *.h); do clang-format -i $$f; done

target:
	mkdir -p $@

target/%: target/%.o $(filter-out $(patsubst %, %.o, $(TARGETS)), $(TARGET_OBJS))
	$(CXX) $(CFLAGS) -o $@ $^

target/%.o: %.cc %.h target
	$(CXX) $(CFLAGS) -c $< -o $@

target/%.o: %.cc target
	$(CXX) $(CFLAGS) -c $< -o $@
