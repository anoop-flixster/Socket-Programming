(a) Full Name --> Anoop Singh


(b) Student ID --> 8466979452


(c) 

I was able to complete Phase 1 & Phase 2 successfully and it works correctly. I completed Phase 3 as well but have not executed that code as it was giving problems in the initial Phase 1 & Phase 2. In the 1st Phase, all the 3 libraries send their book titles and their respective library locations to the Database. The database by the end of this phase, has the complete information about all the books. All this is done over UDP connections. In 2nd Phase, the users query the database and obtain the location of their books. This phase is entirely in UDP as well. In Phase 3, the users connect to the respective libraries where the books are and retrieve the description. This is done over TCP as well. My phase 3 code works in the connection phase but fails eventually. The code for phase 3 is in Library.c -> tcp_library() function and User.c -> tcp_user() function.


(d)

Database.c ->	The Database receives the books info from 3 libraries over UDP in Phase 1. The book locations are returned in the Phase 2.
 
Library.c -> 	The library does everything it's supposed to do as per the project task. The library() function runs for every fork(). The tcp_library() function runs for every fork() of 			phase 3. The code for each library task is encapsulated in separate functions to modularize the task. It enables us to separate the tasks as well. 


User.c ->	The user does everything it's supposed to do as per the project task. The user() function runs for every fork(). The tcp_user() function runs for every fork() of phase	3. The 			code for each library task is encapsulated in separate functions to modularize the task. It enables us to separate the tasks as well. 


We also have the five text files: library1.txt, library2.txt, library3.txt, query1.txt and query2.txt which were given as part of the project.


(e) Steps to run the project
	(i).	The makefile already exists. so:
		(ia)	Use 'gmake' to compile the code
		(ib)	Use 'gmake clean' to gmake afresh

	(ii).	Database must be run:	'Database'

	(iii).	Start a 2nd window. Library must be run:	'Library'

	(iv).	Start a 3rd window. Wait for messages to get over in the 1st 2 windows. User must be run:	'User'

		Phase 1 & 2 should work successfully. 
		Phase 3 : Uncomment the tcp_library() call at the end of library() function in Library.c and tcp_user() call at the end of user() function in User.c
				(I wasn't able to complete Phase 3 successfully. So, commented it out.)


(f) Format of messages exchanged:
	
	User.c -> 	Sent:	Book Title				Sent as title only
			Sent:	Book Title & Library ID			Sent as title#Library_id

	
	Database.c -> 	Sent:	Library ID				Sent as Library_id only
			Sent:	Book Title & Library ID			Sent as title#Library_id


	Library.c -> 	Sent:	Book Title & Library ID			Sent as title#Library_id
			Sent:	Book Description			Sent as book description only


(g) 

The project works fine but will leave zombie processes if the phase 3 code is uncommented. Else, it works perfectly. Sleep() function was used at various locations throughout the code to ensure that the printf statements for different libraries or different users come in sequential fashion.


(h) Beej's code was used for the socket connection part in all the 3 files. 


EXTRA CREDIT / BONUS PART:

	I have used fork() to create threads and it functions correctly satisfying point 15. of the Grading Criteria

	I have submitted a makefile that enables us to compile the code easily without the -lnsl etc. flags satisfying point 16. of the Grading Criteria






