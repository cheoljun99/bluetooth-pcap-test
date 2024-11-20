LDLIBS += -lpcap

all: bluetooth-pcap-test

pcap-test: bluetooth-pcap-test.c

clean:
	rm -f bluetooth-pcap-test *.o
