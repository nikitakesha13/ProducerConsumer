all: UdodenkoN_proj1

run: 
	./UdodenkoN_proj1

UdodenkoN_proj1: UdodenkoN_proj1.c
	gcc -o UdodenkoN_proj1 UdodenkoN_proj1.c -lrt

clean:
	rm -rf *.o UdodenkoN_proj1
