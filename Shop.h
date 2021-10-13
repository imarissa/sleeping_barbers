/**
 * @author  Marissa Christenson
 * @since   05/01/21
 * Shop.h
 * 
 * This is the Shop.h file that defines the Shop class. This class consists 
 * of several private variables, some of which include max_waiting_cust_, the 
 * maximum number of customer threads that are allowed to wait, max_barbers_, 
 * the number of barber threads that will be using the Shop object, 
 * waiting_chairs, the number of customer threads waiting, sleeping_barbers_,  
 * the number of sleeping barber threads. 
 *
 * This class has many private variables that are stated below, as well as 
 * PersonInfo struct that includes information that is unique amongst each 
 * barber. The maximum number of waiting chairs, the number of barbers, the 
 * available barbers, the waiting customer threads, mutex_ and 
 * cond_customers_waiting_ are available to all the threads as public data. 
 * 
 * Each struct that barber_info_ points to is unique to each barber thread 
 * and is only access by the barber thread and its associated customer 
 * thread. It includes the barber id, id of customer in the barber's chair, 
 * info on whether the customer is being serviced, info about barber payments, 
 * a mutex and conditional variables used to synchronize access to this data. 
 **/
#ifndef SHOP_ORG_H_
#define SHOP_ORG_H_
#include <pthread.h>
#include <iostream>
#include <sstream>
#include <string>
#include <queue>
using namespace std;

#define kDefaultNumChairs 3
#define kDefaultNumBarbers 1

class Shop 
{
public:

   /**
    * This is the overloaded constructor that takes an integer for the number of 
    * specified barbers and an integer for the number of specified chairs. 
    * Calls init method to initialize mutex and conditional variables
    * @param num_barbers number of barbers 
    * @param num_chairs maximum number of waiting customers there can be
    * @return none
    * @custom.preconditions  num_barbers >= 1, num_chairs >= 0
    * @custom.postconditions  A Shop object is instantiated
    **/
   Shop(int num_barbers, int num_chairs) : max_barbers_(num_barbers), max_waiting_cust_(num_chairs), cust_drops_(0)
   { 
      init(); 
   };

   /**
    * This is the default constructor that generates a new Shop with default 
    * values, 3 chairs and 1 barber. 
    * Calls init method to initialize mutex and conditional variables
    * @return none
    * @custom.preconditions  none
    * @custom.postconditions  A Shop object is instantiated
    **/
   Shop() : max_barbers_(kDefaultNumBarbers), max_waiting_cust_(kDefaultNumChairs), cust_drops_(0)
   { 
      init();
   };

   /**
    * Destructor for Shop class. 
    * No other methods are called. 
    * @return none
    * @custom.preconditions  none
    * @custom.postconditions  Shop class destroyed. 
    **/
   ~Shop();

   /**
    * This is to be called by the customer threads to visit the shop. They 
    * will check if there is a waiting chair, if not, they leave. If all 
    * barbers are busy and there is a chair, they sit. Otherwise, they sit 
    * in the next available barber chair. 
    * Calls print and int2string methods. 
    * @return id of barber servicing them, -1 if they leave without service
    * @custom.preconditions  none
    * @custom.postconditions  customer thread possibly serviced
    **/
   int visitShop(int id);
   
   /**
    * This is to be called by the customer threads who have been seen by a 
    * barber. Their service will be finished and then they wait before paying. 
    * Calls print method.
    * @return none
    * @custom.preconditions  none
    * @custom.postconditions  customer thread service is completed. 
    **/
   void leaveShop(int customer_id, int barber_id);
   
   /**
    * This is to be called by the barber threads. Barber checks if 
    * there are any customers waiting or in their chair and then service 
    * them. If not, they sleep and wait to be signaled by a customer. 
    * Once they have a customer, they begin the haircut.
    * Calls print method. 
    * @return none
    * @custom.preconditions  none
    * @custom.postconditions  customer thread service is started. 
    **/
   void helloCustomer(int id);
   
   /**
    * This is to be called by the barber threads. This is where they finish 
    * the haircut and take payment from one customer thread and then signal 
    * another waiting customer thread. 
    * Calls print and int2string methods. 
    * @return none
    * @custom.preconditions  none
    * @custom.postconditions  customer thread service is finished
    **/
   void byeCustomer(int id);
   
   /**
    * This returns the number of customers that did not get serviced
    * No other methods are called. 
    * @return number of customers that left shop without being serviced
    * @custom.preconditions  none
    * @custom.postconditions  none
    **/
   int get_cust_drops() const;

 private:

    struct PersonInfo {
      /** unique id of barber */
      int barber_id_{0};
      /** unique id of customer in barber chair, 0 if chair is empty*/
      int cust_in_chair_{0};
      /** Boolean, true if customer paid for service and false otherwise */
      bool money_paid_{false};
      /** Boolean, true if customer is being serviced, false otherwise*/
      bool in_service_{false};
      /** conditional variable related to sleeping barber */
      pthread_cond_t cond_barber_sleeping_ = PTHREAD_COND_INITIALIZER;
      /** conditional variable related to servive payment */
      pthread_cond_t cond_barber_paid_ = PTHREAD_COND_INITIALIZER;
      /** conditional variable related to barber servicing a customer */
      pthread_cond_t cond_cust_served_ = PTHREAD_COND_INITIALIZER;
      /** mutex used to access shared resources within struct */
      pthread_mutex_t mutex_lock_ = PTHREAD_MUTEX_INITIALIZER;
   };

   /** the max number of customer threads that can wait */
   const int max_waiting_cust_;    
   /** the number of barber threads using the Shop object */
   int max_barbers_;
   /** includes the ids of all waiting customer threads */
   queue<int> waiting_chairs_;  
   /** includes the ids of all sleeping barber threads */
   queue<int> sleeping_barbers_;  
   /** number of customer threads not serviced before leaving shop */
   int cust_drops_;

   /** Mutexes and condition variables to coordinate threads*/
   /** pointer to PersonInfo struct with all barber data */
   PersonInfo* barber_info_;
   /** mutex used to enter methods where shared class resources are accessed */
   pthread_mutex_t mutex_;
   /** conditional variables related to waiting customers */
   pthread_cond_t  cond_customers_waiting_;

   /**
    * This initializes the shops mutexes and conditional variables.
    * No other methods are called. 
    * @return none
    * @custom.preconditions  none
    * @custom.postconditions  objects mutexes/ cond variables initialized
    **/
   void init();

   /**
    * This produces a string representation of an integer
    * No other methods are called. 
    * @param i integer to be translated to string 
    * @return none
    * @custom.preconditions  none
    * @custom.postconditions  string representation of integer produced
    **/
   string int2string(int i);

   /**
    * This takes a number and appends ‘barber’ for negative number and 
    * ‘customer’ for positive numbers to a string message that is 
    * printed to stdout.
    * Uses to_string(). No other methods are called. 
    * @param person number indicating person, <0 are barbers, >0 are customers
    * @return none
    * @custom.preconditions  none
    * @custom.postconditions  data sent to stdout
    **/
   void print(int person, string message);

};
#endif
