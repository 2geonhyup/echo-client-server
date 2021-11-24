LDLIBS+=-pthread

all: echo-client echo-server

echo-client: echo-client.o
	$(LINK.cpp) $^ $(LDLIBS) -o $@

echo-server: echo-server.o
	$(LINK.cpp) $^ $(LDLIBS) -o $@

clean:
	rm -f echo-client echo-server *.o