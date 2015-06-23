/*
 * test.cpp
 *
 *  Created on: 23 Jun 2015
 *      Author: lester
 */

//g++ -stdc++11 -lpthread

#include <iostream>
#include <thread>


int thread1(int* pa)
{
	int i;
	for (i=0;i<10000;i++)
	{
		(*pa)++;
	}
	return 0;
}

int main()
{
	int a;
	std::thread t1(thread1,&a);
	std::thread t2(thread1,&a);
	t1.join();
	t2.join();
	std::cout << a << std::endl;
	return 0;

}
