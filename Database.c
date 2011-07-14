
/*
** database.c - database program
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

#define MYPORT "3652"	// the port users will be connecting to for 1st phase
#define MYPORT2 "3552"	// 2nd static port for 2nd phase


#define MAXBUFLEN 500

					// Structure to hold the information in the central database
struct				        
{
	char book_title[50];
	int library_id;
} book[15];

int counter=0;				// Used for going on receive mode 15 times, i.e. 5 times for each library
char *search="#";			// Specifies the token seperator in the packet received
char *token;


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];

	int i;		       		// Used for searching through the structure
	int rv_library_id;		// Stores the library_id where the book_title was found else 0
	int yes=1;			// For the setsockopt
	char lib_buf[MAXBUFLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("database: socket");
			continue;
		}

    /*		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		  {
		    perror("setsockopt");
		    exit(1);
		  }
    */

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("database: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "database: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	//Displaying the addresses
	
	inet_ntop(servinfo->ai_family, get_in_addr((struct sockaddr *)&servinfo->ai_addr), s, sizeof s);
	printf("The central database has UDP port %s and IP address %s\n",MYPORT,s);

	//Receiving book info from the libraries
	
	while(counter < 15)
	{
	  	if (counter == 4 || counter == 9 || counter == 14)			//1 library over
			printf("Received the book list from Library %d\n", book[counter - 1].library_id);

		addr_len = sizeof their_addr;
		if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
			(struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}
	
		buf[numbytes]='\0';
	
		token=strtok(buf, search);					// token should point to the Book Title now 		
		strcpy (book[counter].book_title, buf);
		
		token=strtok(NULL, search);					// token should point to the Library_id now
		book[counter].library_id=atoi(token);
       
    		
		counter++;
	}

      
	close(sockfd);

	printf("\nEnd of Phase 1 for the database\n\n");



	//PHASE 1 ENDS HERE **************************************************************



	memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = AI_PASSIVE; // use my IP

        if ((rv = getaddrinfo(NULL, MYPORT2, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	  	return 1;
        }

        // loop through all the results and bind to the first we can
        for(p = servinfo; p != NULL; p = p->ai_next) {
	  	if ((sockfd = socket(p->ai_family, p->ai_socktype,
			       p->ai_protocol)) == -1) {
	    		perror("database: socket");
	    		continue;
	  	}

	  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
	  {
	  	perror("setsockopt");
		exit(1);
	  }	

	  if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
	    	close(sockfd);
	    	perror("database: bind");
	    	continue;
	  	}

	  break;
        }

        if (p == NULL) {
		  fprintf(stderr, "database: failed to bind socket\n");
		  return 2;
        }

	freeaddrinfo(servinfo);

	
	inet_ntop(servinfo->ai_family, get_in_addr((struct sockaddr *)&servinfo->ai_addr), s, sizeof s);
	printf("The central database has UDP port %s and IP address %s\n",MYPORT2,s);


	counter=0;

	while(counter < 6)					//Listen to total 6 queries, 3 from each user
	  {
	   	addr_len = sizeof their_addr;
	    	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) 
		  {
	      		perror("recvfrom");
	      		exit(1);
	    	}
	      
	    	buf[numbytes]='\0';
	       		
		rv_library_id=0;
										//checking if book exists
		for (i=0; i < 15; i++)
			if (strcmp(buf, book[i].book_title) == 0)
			{
				rv_library_id=book[i].library_id;
				break;
			}

	       		
		sprintf(lib_buf, "%d", rv_library_id);
     
		sleep(0.75);
		
		printf("Sent location info about %s\n", buf);

		if (sendto(sockfd, lib_buf, MAXBUFLEN-1, 0, (struct sockaddr *)&their_addr, addr_len) == -1)		
		{
			perror("sendto");
		        exit(1);
		}
	   
	    	counter++;
	  }


         close(sockfd);

		 printf("\nEnd of Phase 2 for the database\n\n");

		 
	return 0;
}
