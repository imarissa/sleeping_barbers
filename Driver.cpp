/**
 * @author  Marissa Christenson
 * @since   05/01/21
 * Driver.cpp
 * 
 * This is the driver.cpp file that uses the Shop class to display how it 
 * works. It takes the number of barbers, number of waiting chairs, the 
 * number of customers, and the service time for each barber. Creates a shop 
 * object which it passes, along with other information, to the threads it
 * creates. Then it creates the appropriate number of barber and customer 
 * threads which use the Shop object to service customer threads and be 
 * serviced by barber threads, respectively. The customer threads call the 
 * customer method upon being created and the barber threads call barber 
 * methods upon being created. 
 * 
 * The customer threads are waited upon to finish and exit before the barber 
 * threads are then cancelled. 
 * 
 **/
#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include "Shop.h"
using namespace std;

/** method called by barber threads */
void *barber(void *);
/** method called by customer threads */
void *customer(void *);

/**
 * This class represents the thread parameters that are used to identify 
 * the thread and used by the thread. This class is used as a way to pass 
 * more than one argument to a thread. 
*/
class ThreadParam
{
public:
   ThreadParam(Shop* shop, int id, int service_time) :
      shop(shop), id(id), service_time(service_time) {};
      Shop* shop;         
      int id;             
      int service_time;   

};

/**
 * This initializes the shops mutexes and conditional variables.
 * No other methods are called. 
 * @return none
 * @custom.preconditions  none
 * @custom.postconditions  objects mutexes/ cond variables initialized
 **/
int main(int argc, char *argv[]) 
{

   /** arguments read from command line */
   if (argc != 5) {
       cout << "Usage: num_chairs num_customers service_time" << endl;
       return -1;
   }
   int num_barbers = atoi(argv[1]); 
   int num_chairs = atoi(argv[2]);
   int num_customers = atoi(argv[3]);
   int service_time = atoi(argv[4]);

   /** 
    * number of barbers must be >= 1 
    * number of chairs must be >- 0
    * number of customers must be >= 0
    * and service time must be > 0
    */
   if (num_barbers < 1 || num_chairs < 0 || num_customers < 0 || service_time <= 0) {
      cerr << "usage: program2 #barbers, #chair, #customers, #servicetime" << endl;
      cerr << "where #barbers >= 1, #chair >= 1, #customers >= 1, #servicetime > 0" << endl;
      return -1;
   }

   pthread_t barber_thread[num_barbers];
   pthread_t customer_threads[num_customers];
   Shop shop(num_barbers, num_chairs);
  
  /** Create barber threads */
   for (int i = 0; i < num_barbers; i++) {
      usleep(1000);
      ThreadParam* barber_param = new ThreadParam(&shop, i, service_time);
      pthread_create(&barber_thread[i], NULL, barber, barber_param);
   }

   /** Create customer threads */
   for (int i = 0; i < num_customers; i++) {
      usleep(rand() % 1000);
      int id = i + 1;
      ThreadParam* customer_param = new ThreadParam(&shop, id, 0);
      pthread_create(&customer_threads[i], NULL, customer, customer_param);
   }

   /** Wait for customers to finish and cancel barbers */
   for (int i = 0; i < num_customers; i++) {
      pthread_join(customer_threads[i], NULL);
   }

   /** cancel barber threads */
   for (int i = 0; i < num_barbers; i++) {
      //pthread_detach(barber_thread[i]);
      pthread_cancel(barber_thread[i]);
      pthread_join(barber_thread[i], NULL);
   }
   
   cout << "# customers who didn't receive a service = " << shop.get_cust_drops() << endl;
   return 0;
}

/**
 * Called by barber threads that are associated with Shop object to service 
 * customer threads. 
 * Calls the helloCustomer and byeCustomer methods in Shop class.
 * @param arg ThreadParam object with all barber thread info
 * @return none
 * @custom.preconditions  barber details within ThreadParam must be proper
 * @custom.postconditions  barber thread visits shop/ services customer threads
 **/
void *barber(void *arg) 
{
   ThreadParam* barber_param = (ThreadParam*) arg;
   Shop& shop = *barber_param->shop;
   int service_time = barber_param->service_time;
   int id = barber_param->id;
   delete barber_param;

   while(true) {
      shop.helloCustomer(id);
      usleep(service_time);
      shop.byeCustomer(id);
   }
   return nullptr;
}

/**
 * Called by customer threads that use the Shop object to visit the shop 
 * and possibly be serviced. 
 * Calls the visitShop method and may call leaveShop method in Shop class. 
 * @param arg ThreadParam object with all customer thread info
 * @return none
 * @custom.preconditions  customer details within ThreadParam must be proper
 * @custom.postconditions  customer thread visits shop and may be serviced
 **/
void *customer(void *arg)
{
   ThreadParam* customer_param = (ThreadParam*)arg;
   Shop& shop = *customer_param->shop;
   int id = customer_param->id;
   delete customer_param;
   int barber = -1;

   if ((barber = shop.visitShop(id)) != -1) {
      shop.leaveShop(id, barber);
   }
   return nullptr;
}
