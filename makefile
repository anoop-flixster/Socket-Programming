# ANOOP SINGH 8466979457
# Socket Programming Project EE-450 (Session 2)


all: Database Library User

Database: Database.o
	gcc Database.o -o Database -lsocket -lnsl -lresolv

Library: Library.o
	gcc Library.o -o Library -lsocket -lnsl -lresolv

User: User.o
	gcc User.o -o User -lsocket -lnsl -lresolv
	
Database.o: Database.c
	gcc -c Database.c

Library.o: Library.c
	gcc -c Library.c

User.o: User.c 
	gcc -c User.c

clean:
	rm -rf *o Database Library User

