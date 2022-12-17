COMP 304: OPERATING SYSTEMS FALL 2022 – PROJECT 2

				DEADLINE: DECEMBER 17, 2022

ATAKAN ÖZKAN 76277 | DEPARTMENT OF COMPUTER ENGINEERING, KOC UNIVERSITY

BUGRAHAN YAMAN 76070 | DEPARTMENT OF COMPUTER ENGINEERING, KOC UNIVERSITY






















Introduction

In this project, we implemented a deadlock-free simulator which uses multi-threading programming. We implemented five different types of gifts with the different randomizing configuration. For the ElfA which is an operator is implemented to do painting and packaging tasks. For the ElfB which is an also operator is implemented to do the concepts which are required assembly and packaging. We used semaphores to avoid any complication when both are available to do packaging. The last operator is Santa which responsible for delivery and QA tasks. The tasks are printed each second and after all packaged are delivered, there is a print of logs.


main()  
 -> Starts the program.
-> Starts a thread for each of the workers. 
-> Calls CreateOrder() every second until total number of orders are reached.

CreateOrder()
	-> Generates a random number and creates an order according to the probability specified.
	-> Sets the proper variable values for every order.
	-> Creates an order from New Zealand every 30 seconds.

ElfA()
	-> Iterates through the queue and and packages orders if there are any order waiting to be packaged, paints orders if there are any order to be painted, otherwise does nothing.

ElfB()
	-> Iterates through the queue and and packages orders if there are any order waiting to be packaged, assembles orders if there are any order to be assembled, otherwise does nothing.
	







Santa()     

-> Iterates through the queue and and delivers orders if there are any order waiting to be delivered, applies QA on orders if there are any order to be applied QA on, otherwise does nothing.  
ControlThread()     
-> Creates a thread with the given parameter function.  
NOTES:   
  -> We have used only one queue for all types of orders.     -> We have used semaphores in order to prevent ElfA and ElfB to package the same order.     -> We have added variables to Task struct (packaging, painting, assembly, qa, delivery and stage)     -> For the variables packaging, painting, assembly, qa, delivery -1 means no action is necessary, 0 means action is necessary and it is not done yet, 1 means action is necessary and it is already done. -> For the variable stage 0 means it needs to be either assembled, painted or QA’d, 1 means it needs to be packaged and 2 means it needs to be delivered.


















