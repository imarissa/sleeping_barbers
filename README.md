### The Extended Sleeping Barbers Problem
***
##### Specification
***
The Sleeping Barbers problem is an extension of the Sleeping Barber problem. The program attempts to solve the Sleeping Barbers problem where multiple customers visit a barber shop and receive services from one of the available barbers associated with the shop. 

A barbershop consists of a waiting room with `n` waiting chairs and `m` barbers each of which have a barber chair to service customers in `t` time. The barber goes to sleep if there are no customers in the waiting room and/ or in their barber chair. If all the waiting chairs are occupied when a customer enters the shop, then the customer leaves the shop without being serviced by a barber. If the barbers are busy and waiting chairs are available when a customer enters the shop, then the customer sits in one of the free waiting chairs. The customer wakes up the barber and gets served when the barber is asleep due to no existing customers. 

The following command line arguments are required:
| argv[#] | Argument | Description |
| ------ | ------ | ------ |
| argv[1] | num_barbers | The number of barbers working in your barbershop |
| argv[2] | num_chairs | The number of chairs available for customers to wait in |
| argv[3] | num_customers | The number of customers who need a haircut service | 
| argv[4] | service_time | Each barber’s service time (in μ seconds) |

##### Files
***
The Shop.cpp, Shop.h and Driver.cpp are included. The Driver.cpp creates the shop, the barbers and the clients.  It performs the following actions:
* Instantiates a shop which is an object from the Shop class
* Spawns the `n` barbers number of barber threads. Each individual thread is passed a pointer to the shop object (shared), the unique identifier (i.e.  0 ~ num_barbers – 1), and service_time.
* Loops spawning num_customers, waiting a random interval in μ seconds between each new customer being spawned.  All customer threads are passed in a pointer to the shop object and the identifier (i.e., 1 ~ num_customers).
* Waits until all the customer threads are service and terminated.
* Terminates all barber threads.

Languages used: C++

##### Compilation & Running
***
Generate executable:
```sh
g++ Driver.cpp Shop.cpp –o sleepingBarbers –lpthread
```
Run from command line:

```sh
./sleepingBarbers argv1 argv2 argv3 argv4
```
