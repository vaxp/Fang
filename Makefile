CXX = g++
CXXFLAGS = $(shell pkg-config --cflags webkit2gtk-4.1 gtk+-3.0) -Wall -Wextra -O3 -march=native -flto -std=c++11
LIBS = $(shell pkg-config --libs webkit2gtk-4.1 gtk+-3.0) -lsqlite3 -flto
TARGET = vaxp-browser

SOURCES = linux/runner/main.cc \
          linux/runner/database.cc \
          linux/runner/history.cc \
          linux/runner/bookmarks.cc \
          linux/runner/tabs.cc \
          linux/runner/ui.cc \
          linux/runner/adblocker.cc \
          linux/runner/fingerprint_profiles.cc \
          linux/runner/privacy_script.cc
OBJECTS = $(SOURCES:.cc=.o)

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: clean
