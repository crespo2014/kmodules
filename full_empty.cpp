/*
 * full_empty.cpp
 *
 *  There is space in the table to put n plates.
 *  father will fill up the table with plates and notify to child to eat all of them.
 *  when child finish it will notify to father.
 *
 *  1. fill up to top the table
 *  2. clean all
 *  3. repeat
 *
 *  Created on: 25 Jun 2015
 *      Author: lester
 *
 *  g++ -g -lpthread -lrt full_empty.cpp -o full_empty
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
  char plates[20];
  std::atomic_short count;    // si no es zero no lo toca le productor, si es zero no lo toca el consumidor ( simple candado hecho a mano)
  std::mutex mtx;
  std::condition_variable cnd;
};

/*
 * no necesitamos candados aqui
 * esta funcion no se detendra pro nada, si no hay datos no hace nada
 * si hay datos los procesa, no hay candado, tenemos una garantia de ejecucion cada segundo ( aproximadamente )
 */
void child(struct table_t* t, int id)
{
  char  c;
  while (t->count < sizeof(t->plates) + 1)
  {
    // try to eat
    if (t->count != 0)
    {
      c = t->plates[t->count-1];  // lee antes de decrementar, nadie va a tocar esto si no es zero
      t->count.fetch_sub(1);      // decrementa . ( si se pone en zero seguro el padre lo toca
      if (t->count ==0)
        t->cnd.notify_one();    // avisale al padre
    }
    std::cout << c << std::endl;    // usa el dato obtenido
    sleep(1);
  }
  std::cout << "end of "<< id << std::endl;
}

int main()
{
  struct table_t tbl;
  tbl.count = 0;
  std::thread t(child,&tbl,1);

  std::unique_lock < std::mutex > lock(tbl.mtx); // close door no lo necesitamos pero asi fucniona el sistema
  memcpy(tbl.plates,"ABCDEFGHIJKLMN",10);
  tbl.count = 10;
  tbl.cnd.wait(lock, [&tbl] { return tbl.count == 0;});     // esperar

  memcpy(tbl.plates,"12345678901234567",16);
  tbl.count = 16;
  tbl.cnd.wait(lock, [&tbl] { return tbl.count == 0;});     // esperar

  tbl.count = 25;   // en el proximo segundo el hilo se muere
  t.join();
  return 0;

}

