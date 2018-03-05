
TESTS := queue darray MakeSound

LDLIBS := -lcgreen
OBJS := ../objs
CFLAGS += -I../src

test: $(TESTS)
	@for t in $(TESTS); do ./$$t; done

queue: $(OBJS)/queue.o
darray: $(OBJS)/darray.o

$(OBJS)/%.o:
	$(MAKE) -C ../ $(subst ../,,$@)

$(TESTS): %: %-test.c main.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS) 

clean:
	rm -f $(TESTS) *.o
