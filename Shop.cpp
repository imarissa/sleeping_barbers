/**
 * @author  Marissa Christenson
 * @since   05/01/21
 * Shop.cpp
 * 
 * This is the Shop.cpp file that is implements the methods of the Shop class. 
 * It includes several public methods, a default constructor and an overloaded 
 * constructor that takes the number of barbers and the number of waiting 
 * chairs. visitShop and leaveShop are called by customer threads to visit 
 * the shop and be serviced. If there are no waiting chairs the customer 
 * threads leave. Otherwise, it stays and waits or sits in a barber chair 
 * to be serviced. The helloCustomer and byeCustomer are used by the barber 
 * threads to service the customer threads and then process them before they 
 * leave. 
 * 
 * The private methods are used to initialize the shop variable and to turn 
 * integers to string/ to print data to stdout. 
 **/
#include "Shop.h"

/**
 * This initializes the shops mutexes and conditional variables.
 * No other methods are called. 
 * @return none
 * @custom.preconditions  none
 * @custom.postconditions  objects mutexes/ cond variables initialized
 **/
void Shop::init() 
{
   pthread_mutex_init(&mutex_, NULL);
   pthread_cond_init(&cond_customers_waiting_, NULL);
   barber_info_ = new PersonInfo[max_barbers_];
   for(int i = 0; i < max_barbers_; i++) {
      barber_info_[i].barber_id_ = i;
      sleeping_barbers_.push(i);
   }
}

/**
 * Destructor for Shop class. 
 * No other methods are called. 
 * @return none
 * @custom.preconditions  none
 * @custom.postconditions  Shop class destroyed. 
 **/
Shop::~Shop() 
{
   delete[] barber_info_;
}

/**
 * This initializes the shops mutexes and conditional variables.
 * No other methods are called. 
 * @return none
 * @custom.preconditions  none
 * @custom.postconditions  objects mutexes/ cond variables initialized
 **/
string Shop::int2string(int i) 
{
   stringstream out;
   out << i;
   return out.str( );
}

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
void Shop::print(int person, string message)
{
   cout << ((person > 0) ? "customer[" + to_string(person) : "barber  [" + to_string((person * -1))) << "]: " << message << endl;
}

/**
 * This returns the number of customers that did not get serviced
 * No other methods are called. 
 * @return number of customers that left shop without being serviced
 * @custom.preconditions  none
 * @custom.postconditions  none
 **/
int Shop::get_cust_drops() const
{
    return cust_drops_;
}

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
int Shop::visitShop(int id)
{
   pthread_mutex_lock(&mutex_);
   
   /** If all chairs are full then leave shop */
   if (max_waiting_cust_ > 0) {
      /** If someone is being served or transitioning waiting to service
       * then take a chair and wait for service chair */
      if (waiting_chairs_.size() == max_waiting_cust_) {
         print(id,"leaves the shop because of no available waiting chairs.");
         ++cust_drops_;
         pthread_mutex_unlock(&mutex_);
         return -1;
      } else {
         if (sleeping_barbers_.size() == 0 || !waiting_chairs_.empty()) {
            waiting_chairs_.push(id);
            print(id, "takes a waiting chair. # waiting seats available = " + 
            int2string(max_waiting_cust_ - waiting_chairs_.size()));
            pthread_cond_wait(&cond_customers_waiting_, &mutex_);
            waiting_chairs_.pop();
         }
      }
   }else {
      if (sleeping_barbers_.size() == 0) {
         print(id,"leaves the shop because of no available barbers.");
         ++cust_drops_;
         pthread_mutex_unlock(&mutex_);
         return -1;
      }
   }
   
   int barber_id = sleeping_barbers_.front();
   sleeping_barbers_.pop();

   print(id, "moves to a service chair[" + int2string(barber_id) + "]" 
   + ". # waiting seats available = " 
   + int2string(max_waiting_cust_ - waiting_chairs_.size()));

   pthread_mutex_unlock(&mutex_); 
   pthread_mutex_lock(&(barber_info_[barber_id].mutex_lock_));

   barber_info_[barber_id].cust_in_chair_ = id;
   barber_info_[barber_id].in_service_ = true;

   /** Wake up the barber in case he is sleeping */
   pthread_cond_signal(&(barber_info_[barber_id].cond_barber_sleeping_));
   pthread_mutex_unlock(&(barber_info_[barber_id].mutex_lock_));
   return barber_id;
}

/**
 * This is to be called by the customer threads who have been seen by a 
 * barber. Their service will be finished and then they wait before paying. 
 * Calls print method.
 * @return none
 * @custom.preconditions  none
 * @custom.postconditions  customer thread service is completed. 
 **/
void Shop::leaveShop(int customer_id, int barber_id)
{
   pthread_mutex_lock(&(barber_info_[barber_id].mutex_lock_));
   /** Wait for service to be completed */
   print(customer_id, "waits for barber[" + int2string(barber_id) + "]" + " to be done with hair-cut");
   
   while (barber_info_[barber_id].in_service_ == true) {
      pthread_cond_wait(&(barber_info_[barber_id].cond_cust_served_), &(barber_info_[barber_id].mutex_lock_));
   }

   /** Pay the barber and signal barber appropriately */
   barber_info_[barber_id].money_paid_ = true;
   pthread_cond_signal(&(barber_info_[barber_id].cond_barber_paid_));
   print(customer_id, "says good-bye to the barber[" + int2string(barber_id) + "]");
   pthread_mutex_unlock(&(barber_info_[barber_id].mutex_lock_));
}
 
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
void Shop::helloCustomer(int id)
{
   pthread_mutex_lock(&mutex_);
   pthread_mutex_lock(&(barber_info_[id].mutex_lock_));

   /** If no customers then barber can sleep */
   if (waiting_chairs_.empty() && barber_info_[id].cust_in_chair_ == 0) {
      print(0 - id, "sleeps because of no customers.");
      pthread_mutex_unlock(&mutex_);
      pthread_cond_wait(&(barber_info_[id].cond_barber_sleeping_), &(barber_info_[id].mutex_lock_));
   }

   /** check if the customer sat in barber chair. */
   if (barber_info_[id].cust_in_chair_ == 0) {
      pthread_mutex_unlock(&mutex_);
      //cout << "--------------Barber sleeps again because nobody in chair: ------------------" << id <<endl;
      pthread_cond_wait(&(barber_info_[id].cond_barber_sleeping_), &(barber_info_[id].mutex_lock_));
   }else {
      pthread_mutex_unlock(&mutex_);
   }
   print(0 - id, "starts a hair-cut service for customer[" + int2string(barber_info_[id].cust_in_chair_) + "]");
   pthread_mutex_unlock(&(barber_info_[id].mutex_lock_));
}

/**
 * This is to be called by the barber threads. This is where they finish 
 * the haircut and take payment from one customer thread and then signal 
 * another waiting customer thread. 
 * Calls print and int2string methods.  
 * @return none
 * @custom.preconditions  none
 * @custom.postconditions  customer thread service is finished
 **/
void Shop::byeCustomer(int id)
{
   pthread_mutex_lock(&(barber_info_[id].mutex_lock_));

  /** Hair Cut-Service is completed, signal customer and wait for payment */
  print(0 - id, "says he's done with a hair-cut service for customer[" + int2string(barber_info_[id].cust_in_chair_) + "]");
  
  barber_info_[id].in_service_ = false;
  barber_info_[id].money_paid_ = false;
  pthread_cond_signal(&(barber_info_[id].cond_cust_served_));
  
  while (barber_info_[id].money_paid_ == false) {
      pthread_cond_wait(&(barber_info_[id].cond_barber_paid_), &(barber_info_[id].mutex_lock_));
  }

  barber_info_[id].cust_in_chair_ = 0;
  /** Signal to customer to get next one */
  print(0 - id, "calls in another customer");
  pthread_mutex_unlock(&(barber_info_[id].mutex_lock_));
  pthread_mutex_lock(&mutex_);
  sleeping_barbers_.push(id);
  pthread_cond_signal(&cond_customers_waiting_);
  pthread_mutex_unlock(&mutex_);
}
