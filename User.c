/*
** user.c - The user program
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVERPORT "3552"					// the port users will be connecting to

								// the 3 respective port numbers for TCP connections, 1 for each library
#define LIBRARY1PORT "21552"
#define LIBRARY2PORT "21652"
#define LIBRARY3PORT "21752"

#define MAXBUFLEN 500

#define BACKLOG 10    

void user(int);							// Declaration
void tcp_user(int);

struct
{
	char book_title[50];
	char book_description[300];
  	int library_id;
} book[3];


struct hostent *hp;    						// Used to store the local host's IP Address using gethostbyname()

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
    		return &(((struct sockaddr_in*)sa)->sin_addr);
  	}

  	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
  	int  pid1, pid2;     		       			// One for each of the 2 forked processes
	int status;	
	
	int i;							// Used as an index when forking the 2 users
       
	if (argc != 1)
	{
        	fprintf(stderr,"usage: user\n");
		exit(1);
	}
       
	if ( (pid1=fork() ) == 0)      				// Forking 1st User process 
	  {	
		user(1);
		return 0;
	}
	else
	  {
	 //    	sleep(1.00);		

	        if (pid1 == -1)					// Process Creation Failed. STOP !!
	        {
		      fprintf(stderr,"Couldn't 'fork()', i.e. create a user. Error !!\n");
		      return 1;
	        }

		if ( (pid2=fork() ) == 0 )			// Forking 2nd user Process
		{
			user(2);
			return 0;
		}
	        else
		{
		  //  	sleep(1.00);
			
		        if (pid2 == -1)                         // Process Creation Failed. STOP !!
			{
                        	fprintf(stderr,"Couldn't 'fork()', i.e. create a user. Error !!\n");
                      		return 1;
		    	}
      
							// Waiting for the 3 libraries to signal
			if (waitpid(pid1, &status, 0) == -1 || waitpid(pid2,&status, 0) == -1)
			{
		        	fprintf(stderr,"Couldn't 'waitpid()' correctly. Error !!\n");
				return 1;
		        }
    
			return 0;
		}   
		
	}    

	
}


void user(int user_id)
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;

	struct sockaddr_in *socket_name;			// Used for the getsockname() method
	int sin_size, *sin_size1;			        // Used for the getsockname() method
	char *sip_addr;					      	// For the Socket Name

	char *msg;						// Contains the message finally sent in the packet
	char message[500];					// A buffer character array, which stores the line read from the file
	FILE *fp_in;						// File Pointer for the user[1/2].txt file
	char host_name[200];		       			// Used in gethostname() & gethostbyname() to obtain the IP Address
	struct in_addr **addr_list;				// Used to obtain the IP address of local host
	int i;
	int counter;						// Used for the Structure's index
	struct sockaddr_storage their_addr;			// Used for the recvfrom() method	
        socklen_t addr_len;					// Used fro the recvfrom() method

	counter=0;						// Initializing value to 0

        memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	sin_size=sizeof (socket_name);				// Stores the sizeof a sockaddr_in
   	sin_size1=&(sin_size);

	if ((rv = getaddrinfo("localhost", SERVERPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("user: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			perror("library: connect");
			continue;
		}	 
		
		break;
	}


	if (p == NULL) {
	  fprintf(stderr, "user: failed to bind socket\n");
	  exit(1);
        }

       gethostname(host_name, sizeof (host_name)); 

	hp=gethostbyname(host_name);                          // hp now stores the IP address of the local host
        if (hp == NULL)                                       // Making sure that gethostbyname() worked correctly
	{
	  	fprintf(stderr, "Couldn't run gethostbyname() correctly. Error !!\n");
	  	exit(1);
	}

		
	if (getsockname(sockfd, (struct sockaddr *)socket_name, sin_size1 ) == -1)	// Used to obtain the socket's name
	{
		fprintf(stderr, "Couldn't run getsockname() correctly. Error !!\n");	//Error checking for getsockname()
		exit(1);
	}
	else							// getsockname() worked correctly
	{
		printf("User %d has UDP port %d and IP address ", user_id, socket_name -> sin_port); 
	  	addr_list=(struct in_addr **)hp -> h_addr_list;
	  	
		for (i=0; addr_list[i] != NULL; i++)
	    		printf("%s ", inet_ntoa(*addr_list[i]));

	        printf("\n");
	}
	

	//Message Preparation code to: 
	//1. Use fgets() to read a line of text from the user text file
	
	switch(user_id)	       				// Open the respective library file
	{
	case 1: fp_in=fopen("queries1.txt", "r");
			break;  
		
	case 2: fp_in=fopen("queries2.txt", "r"); 
	  		break;
	}
	
	if (fp_in == NULL)					// Check if the file was opened successfully or not
	{
	  fprintf(stderr, "Couldn't run fopen() correctly. Error !!\n");
	  exit(1);
	}
	
	printf("\n");
	

       	while(fgets(message, sizeof (message), fp_in))
	  {    
      		msg=strtok(message, "\n");			// Now, msg contains the Book Name
    
		strcpy(book[counter].book_title, msg);
		printf("Checking %s in the database\n", msg);
      
	       
		if ((numbytes += send(sockfd, msg , strlen(msg), 0)) == -1) 
		{
			perror("user: sendto");
			exit(1);
		}

	    
		addr_len=sizeof their_addr;
		if (recvfrom(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&their_addr, &addr_len) == -1)
		{
		  	perror("user: recfrom");
			exit(1);
		}
		
		printf("Received location info of %s from the database\n", book[counter].book_title);

		book[counter++].library_id=atoi(msg);

	}

	sleep(1.00);

	printf("\nCompleted book queries to the database from User  %d\n\n", user_id);

	fclose(fp_in);



	freeaddrinfo(servinfo);
	
     	close(sockfd);


	printf("End of Phase 2 for User %d\n\n", user_id);

	sleep(1.00);

	  sleep(10.0);
	  
//	tcp_user(user_id);

}


void tcp_user(int user_id)
{
	int sockfd, sockfd1, sockfd2; // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	FILE *fp_in;

	int numbytes;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;
	int i;

	char *search="#";						// Specifies the token seperator in the packet received
	char *token;
	char *buf;
	char *lib_buf;
	char *msg;
	char message[500];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;


	if ((rv = getaddrinfo("localhost", LIBRARY1PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
       		if ((sockfd = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
			perror("user: socket");
			continue;
		}
		/*
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
			sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}
		*/

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("user: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "user: failed to bind\n");
		exit(1);
	}

	freeaddrinfo(servinfo); // all done with this structure

  
      	for(i=0; i < 3; i++)
    		if (book[i].library_id == 1)
      		{		
      			strcpy(lib_buf,book[i].book_title);		// msg contains the book title now					
       			switch(user_id)
      			{
      				case 1: strcat(msg, "#1");
      				 	break;

      				case 2: strcat(msg, "#2");
      			  		break;
      			}

			printf("\nI meant to send this -> %s %d %d\n", lib_buf, user_id, book[i].library_id);

			//	sleep(2.0);	//tryin sth new
			
      	 		if ((numbytes=send(sockfd, lib_buf , strlen(lib_buf), 0)) == -1)
       	 		{
       	 			perror("user: sendto"); 
   	      			exit(1);
      	 		}
		 
			printf("numbytes: %d\n", numbytes);

     			if (numbytes = recv(sockfd, buf, strlen(buf), 0) == -1)
      			{	
      				perror("recv");
     			    	exit(1);
    			}

      			buf[numbytes]='\0';

			printf("I received this -> %s %d\n", buf, user_id);

      			strcpy(book[i].book_description,buf);
	       }
			
		close(sockfd);						// parent doesn't need this

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	if ((rv = getaddrinfo("localhost", LIBRARY2PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
       		if ((sockfd1 = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
			perror("user: socket");
			continue;
		}
		/*
		if (setsockopt(sockfd1, SOL_SOCKET, SO_REUSEADDR, &yes,
			sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}
		*/

		if (connect(sockfd1, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd1);
			perror("user: bind");
			continue;
		}

		break;
	}

  
	if (p == NULL) {
		fprintf(stderr, "user: failed to bind\n");
		exit(1);
	}

     
	freeaddrinfo(servinfo); // all done with this structure


      	for(i=0; i < 3; i++)
    		if (book[i].library_id == 2)
      		{		
      			strcpy(lib_buf,book[i].book_title);		// msg contains the book title now					
       			switch(user_id)
      			{
      				case 1: strcat(msg, "#1");
      				 	break;

      				case 2: strcat(msg, "#2");
      			  		break;
      			}

			printf("\nI meant to send this -> %s %d %d\n", lib_buf, user_id, book[i].library_id);

      	 		if ((numbytes=send(sockfd, lib_buf , strlen(lib_buf), 0)) == -1)
       	 		{
       	 			perror("user: sendto"); 
   	      			exit(1);
      	 		}

			printf("numbytes: %d\n", numbytes);
      			
     			if (numbytes = recv(sockfd1, buf, strlen(buf), 0) == -1)
      			{
      				perror("recv");
     			    	exit(1);
    			}

      		     
      			buf[numbytes]='\0';

			printf("I received this -> %s %d\n", buf, user_id);

      			strcpy(book[i].book_description,buf);
	       }
			
		close(sockfd1);						// parent doesn't need this

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
       
	if ((rv = getaddrinfo("localhost", LIBRARY3PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
       		if ((sockfd2 = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
			perror("user: socket");
			continue;
		}
		
		if (setsockopt(sockfd2, SOL_SOCKET, SO_REUSEADDR, &yes,
			sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}
		

		if (connect(sockfd2, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd2);
			perror("user: bind");
			continue;
		}

		break;
	}

    
	if (p == NULL) {
		fprintf(stderr, "user: failed to bind\n");
		exit(1);
	}

	
	freeaddrinfo(servinfo); // all done with this structure


      	for(i=0; i < 3; i++)
    		if (book[i].library_id == 3)
      		{		
      			strcpy(lib_buf,book[i].book_title);		// msg contains the book title now					
       			switch(user_id) 
      			{
      				case 1: strcat(msg, "#1");
      				 	break;

      				case 2: strcat(msg, "#2");
      			  		break;
      			}
 
			printf("\nI meant to send this -> %s %d %d\n", lib_buf, user_id, book[i].library_id);

      	 		if ((numbytes=send(sockfd, lib_buf , strlen(lib_buf), 0)) == -1)
       	 		{
       	 			perror("user: sendto"); 
   	      			exit(1);
      	 		}

			printf("numbytes: %d\n", numbytes);
      		      
     			if (numbytes = recv(sockfd2, buf, strlen(buf), 0) == -1)
      			{
      				perror("recv");
     			    	exit(1);
    			}

			printf("I received this -> %s %d\n", buf, user_id);
      		      
      			buf[numbytes]='\0';

      			strcpy(book[i].book_description,buf);
	       }
			
		close(sockfd2);						// parent doesn't need this
		
	exit(0);

}
