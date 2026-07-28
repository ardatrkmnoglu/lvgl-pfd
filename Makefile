LV_DIR = /home/ardatrkmnoglu/Downloads/lv_port_linux/lvgl/src
BUILD_DIR = bin

CC = gcc
CFLAGS = -I./ -I$(LV_DIR) -O3 -lSDL2 -lm -lstdc++ $(shell pkg-config --cflags freetype2) $(shell pkg-config --libs freetype2)

LV_SRCS = $(shell find $(LV_DIR) -name "*.c" -o -name "*.cpp")
APP_SRCS = pfd.c

SRCS = $(APP_SRCS) $(LV_SRCS)
OBJS = $(SRCS:.c=.o)

TARGET = $(BUILD_DIR)/pfd_sim

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all run clean
