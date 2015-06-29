/*
 * atomic.cpp
 *
 *  Created on: 26 Jun 2015
 *      Author: lester
 *
 *      for i in 0 1 2 3; do g++ -std=c++11 -O$i -lpthread -lrt atomic.cpp -o atomic_$i; done
 *
 */



#include <iostream>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <mutex>
#include <condition_variable>
#include <atomic>

struct table_t
{
  // volatile bool go;
  //unsigned counter;
  volatile bool go;
  std::atomic_ulong counter;
};


void child(struct table_t* t, int id)
{
  while(!t->go);      // wait
  for(int i = 0; i<50000;i++)
    t->counter++;
}

int main()
{
  struct table_t tbl;
  tbl.counter = 0;
  tbl.go = false;
  std::thread t1(child,&tbl,1);
  std::thread t2(child,&tbl,1);
  tbl.go = true;
  t1.join();
  t2.join();
  std::cout << tbl.counter << std::endl;
  return 0;
}
