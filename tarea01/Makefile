
SOURCES=simulator.cpp prototype.cpp statistics.cpp
OBJECTS=$(patsubst %.cpp,%.o,$(SOURCES))
OUTPUT=simulator
DEPENDENCIES=gstreamer-1.0 gstreamer-app-1.0 gstreamer-video-1.0

.PHONY: all clean

all: simulator

%.o : %.cpp
	@g++ -g3 -O0 -c $< -o $@ `pkg-config --cflags $(DEPENDENCIES)`

$(OUTPUT): $(OBJECTS)
	@g++ $^ -o $@ `pkg-config --libs $(DEPENDENCIES)`

clean:
	@rm -f $(OBJECTS) $(OUTPUT) *~ \#*
