DEPS =
OBJ = demo.o
CFLAGS =
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
demo: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
.PHONY: clean
clean:
	rm -f *.o *~ demo
.PHONY: install
install:
	cp -f demo $(TARGET_DIR)/usr/bin/
.PHONY: uninstall
uninstall:
	rm -f $(TARGET_DIR)/usr/bin/demo

