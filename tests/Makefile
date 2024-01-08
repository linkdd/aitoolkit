DESTDIR = ../build/tests/

CXXFLAGS := -std=c++23 -O2 -g
SOURCES = $(wildcard *.cpp)
TARGET = aitoolkit-test-runner

.PHONY: all
all:
	@echo "  CXX     $(TARGET)"
	@mkdir -p $(DESTDIR)
	@$(CXX) $(CXXFLAGS) $(SOURCES) -o $(DESTDIR)/$(TARGET)
	@echo "  RUN     $(TARGET)"
	@exec $(DESTDIR)/$(TARGET)
