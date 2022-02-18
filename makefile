all: UdodenkoN_proj1

run: 
	./UdodenkoN_proj1.exe

UdodenkoN_proj1: UdodenkoN_proj1.c
	gcc -o UdodenkoN_proj1.exe UdodenkoN_proj1.c -lpthread -lrt

clean:
	rm -rf *.exe
