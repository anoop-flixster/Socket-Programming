/*
** library.c - The library program
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
#include <signal.h>
#include <sys/wait.h>

#define SERVERPORT "3652"					// the port users will be connecting to

#define LIBRARY1PORT "21552"					// 3 Static ports for each Library for Phase 3
#define LIBRARY2PORT "21652"
#define LIBRARY3PORT "21752"

#define MAXBUFLEN 500

#define BACKLOG 10						// how many pending connections queue will hold

void library(int);						// Declaration
void tcp_library(int);

struct hostent *hp;    						// Used to store the local host's IP Address using gethostbyname()

void sigchld_handler(int s)
{
  	while(waitpid(-1, NULL, WNOHANG) > 0);
}

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
  	int  pid1, pid2, pid3;     		       		// One for each of the 3 forked processes
	int status;	
	
	int i;							// Used as an index when forking the 3 libraries
       		
	if (argc != 1)
	{
        	fprintf(stderr,"usage: library\n");
		exit(1);
	}

	if ( (pid1=fork() ) == 0)      				// Forking 1st Library process 
	{	
		library(1);
		return 0;
	}
	else
	  {
	    	sleep(1.00);		

	        if (pid1 == -1)					// Process Creation Failed. STOP !!
	        {
		      fprintf(stderr,"Couldn't 'fork()', i.e. create a library. Error !!\n");
		      return 1;
	        }

		if ( (pid2=fork() ) == 0 )			// Forking 2nd Library Process
		{
			library(2);
			return 0;
		}
	        else
		{
		  	sleep(1.00);
			
		        if (pid2 == -1)                         // Process Creation Failed. STOP !!
			{
                        	fprintf(stderr,"Couldn't 'fork()', i.e. create a library. Error !!\n");
                      		return 1;
		    	}
      
			if ( (pid3=fork() ) == 0)		// Forking 3rd Library Process
			{
				library(3);
				return 0;
			}
        		else
			  {	
			  	if (pid3 == -1)                  // Process Creation Failed. STOP !!
			        {
					fprintf(stderr,"Couldn't 'fork()', i.e. create a library. Error !!\n");
					return 1;
			        }
		
								// Waiting for the 3 libraries to signal
				if (waitpid(pid1, &status, 0) == -1 || waitpid(pid2,&status, 0) == -1 || waitpid(pid3,&status, 0) == -1)
				{
					fprintf(stderr,"Couldn't 'waitpid()' correctly. Error !!\n");
					return 1;
				}
    
				return 0;
			  }   
		}
	}    

	
}


void library(int library_id)
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
	FILE *fp_in;						// File Pointer for the library[1/2/3].txt file
	char host_name[200];		       			// Used in gethostname() & gethostbyname() to obtain the IP Address
	struct in_addr **addr_list;				// Used to obtain the IP address of local host
	int i;
	int yes=1;
	
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
			perror("talker: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			perror("library: connect");
			continue;
		}	 
		
		
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
			       sizeof(int)) == -1) {
		 	 perror("setsockopt");
		  	exit(1);
                }
			
		break;
	}


	if (p == NULL) {
	  fprintf(stderr, "library: failed to bind socket\n");
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
		printf("Library %d has UDP port %d and IP address ", library_id, socket_name -> sin_port); 
	  	addr_list=(struct in_addr **)hp -> h_addr_list;
	  	
		for (i=0; addr_list[i] != NULL; i++)
	    		printf("%s ", inet_ntoa(*addr_list[i]));

	        printf("for phase 1\n");
	}
	
	

	//Message Preparation code to: 
	//1. Use fgets() to read a line of text from the library text file
	//2. Use strtok() to seperate the book name and description seperated by '#'

	switch(library_id)	       				// Open the respective library file
	{
	case 1: fp_in=fopen("library1.txt", "r");
			break;  
		
	case 2: fp_in=fopen("library2.txt", "r"); 
		  	break;

	case 3: fp_in=fopen("library3.txt", "r"); 
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
		msg=strtok(message, "#");			// Seperates the book name and description. Now, msg contains the Book Name
	
		printf("Library %d has sent %s to the database\n", library_id, msg);
      
		switch(library_id)				// library_id appended after the # seperator token
		{
			case 1: strcat(msg, "#1");
			break;

        		case 2: strcat(msg, "#2");
	  		break;

        		case 3: strcat(msg, "#3");
	  		break;
		}
	       
		if ((numbytes += send(sockfd, msg , strlen(msg), 0)) == -1) 
		{
			perror("library: sendto");
			exit(1);
		}

	}

	printf("\nUpdating the database is done for Library %d\n\n", library_id);

	fclose(fp_in);



	freeaddrinfo(servinfo);
	
     	close(sockfd);


	printf("End of Phase 1 for library %d\n\n\n", library_id);

	sleep(2.0);
       
//      	tcp_library(library_id);
	
}

void tcp_library(int library_id)
{
	int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	FILE *fp_in;

	int childID;
	
	int numbytes;
	int user_id;
	int yes;
	struct sigaction sa;
	char s[INET6_ADDRSTRLEN];
	int rv;

	char *search="#";						// Specifies the token seperator in the packet received
	char *token;
	char buf[MAXBUFLEN];
	char lib_buf[MAXBUFLEN];
	char *msg;
	char message[500];
	
       
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	
	switch(library_id)
	{
		case 1:
               		if ((rv = getaddrinfo(NULL, LIBRARY1PORT, &hints, &servinfo)) != 0) {
					fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
					exit(1);
				}

				break;


		case 2:
       			if ((rv = getaddrinfo(NULL, LIBRARY2PORT, &hints, &servinfo)) != 0) {
					fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
					exit(1);
				}

				break;


        	case 3:
		        if ((rv = getaddrinfo(NULL, LIBRARY3PORT, &hints, &servinfo)) != 0) {
					fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
					exit(1);
				}

					break;
		
	}

	
	yes=1;
	
	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
       		if ((sockfd = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
			perror("library: socket");
			continue;
		}
	
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
			sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		printf("Reached before bind\n");
		
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("library: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "library: failed to bind\n");
		exit(1);
	}

	freeaddrinfo(servinfo); // all done with this structure

    sa.sa_handler=sigchld_handler;					// reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags=SA_RESTART;

	if(sigaction(SIGCHLD, &sa, NULL) == -1)
	{
	  	perror("sigaction");
	  	exit(1);
	}

       
	while(1)							// main accept() loop
	{
		if (listen(sockfd, BACKLOG) == -1) {
			perror("listen");
			exit(1);
		}
	
		sin_size=sizeof their_addr;
		
	       
		new_fd=accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

		if (new_fd == -1)
		{
			perror("accept");
			continue;
		}

		
		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);

		printf("library: got connection from %s %d\n", s, library_id);
 
		if (childID = fork() == 0)						// this is the child process
		{
		while(1){				//new try
//		  	close(sockfd);					// child doesn't need the listener
			if (numbytes=recv(new_fd, buf, sizeof buf, 0) == -1)
			{
				perror("recv");
				exit(1);
			}
 
		   
       		        buf[numbytes]='\0';

			printf("I received this -> %s d %d d %d\n", buf, library_id, numbytes);
       
			token=strtok(buf, search);                              // token should point to the Book Title now

			token=strtok(NULL, search);                             // token should point to the User_id now
                        user_id=atoi(token);

	      		switch(library_id)                                      // Open the respective library file
              	    	{
	     	    		case 1: fp_in=fopen("library1.txt", "r");
      		      			break;

     		    		case 2: fp_in=fopen("library2.txt", "r");
		      			break;

     		    		case 3: fp_in=fopen("library3.txt", "r");
    			      		break;

   	      	    	}

       		  	if (fp_in == NULL)                                      // Check if the file was opened successfully or not
  	      	    	{
	      	      		fprintf(stderr, "Couldn't run fopen() correctly. Error !!\n");
       		      		exit(1);
      		    	}

				  
      		  	while(fgets(message, sizeof (message), fp_in))
				{
		        	msg=strtok(message, "#");			// msg should now point to the Book title  
	
	     			if (strcmp(msg, buf) == 0)     			// Book title found
	      			{
	      				msg=strtok(NULL, ".");			// msg contains Book description now
	      				break;
	      			}
     			}


			sprintf(lib_buf, "%s", msg);
			
			printf("I sent this -> %s %d\n", lib_buf, library_id);		    

			if (send(new_fd, lib_buf, MAXBUFLEN-1, 0) == -1)
			{
				perror("send");
			   	exit(1);
	     		} 

		      
	//		close(new_fd);
	//		exit(0);
		}
		}
		else
			wait(&childID);
		close(new_fd);						// parent doesn't need this
	}
	
	close(sockfd);					// child doesn't need the listener
	exit(0);

}
