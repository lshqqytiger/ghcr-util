CC = g++
CFLAGS = -Wall
LDFLAGS =
OBJS = $(patsubst %.cpp,%.o,$(wildcard *.cpp))
DEPS = ./json/json.h
TARGET = main

main: $(OBJS)
	@$(CC) -o $@ $^ $(LDFLAGS)

$(OBJS): %.o: %.cpp
	@$(CC) $(CFLAGS) -o $@ -c $<

%.d: %.cpp
	@$(CC) -MM $< > $@

-include $(patsubst %.cpp,%.d,$(wildcard *.cpp))

clean:
	@rm -f $(OBJS) $(TARGET)

test:
	@./main
