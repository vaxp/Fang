CXX = g++
CXXFLAGS = $(shell pkg-config --cflags webkit2gtk-4.1 gtk+-3.0) -Wall -Wextra -O3 -march=native -flto -std=c++11
LIBS = $(shell pkg-config --libs webkit2gtk-4.1 gtk+-3.0) -lsqlite3 -flto
TARGET = vaxp-browser

SOURCES = fang/main.cc \
          fang/database.cc \
          fang/history.cc \
          fang/bookmarks.cc \
          fang/tabs.cc \
          fang/ui.cc \
          fang/adblocker.cc \
          fang/fingerprint_profiles.cc \
          fang/privacy_script.cc \
          fang/tracker_domains.cc \
          fang/network_blocker.cc \
          fang/adblockplus_integration.cc
OBJECTS = $(SOURCES:.cc=.o)

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: clean update-adblock

update-adblock:
	python3 tools/update_adblock.py
