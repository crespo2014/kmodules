/*
 * test.cpp
 *
 *  Created on: 23 Jun 2015
 *      Author: lester
 */

//g++ -stdc++11 -lpthread
#include <iostream>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <mutex>
#include <condition_variable>

struct buffer_t
{
	char data[100];
	char* start;
	char* end;
	bool go;
};

int fastconsumer(buffer_t* p,int id)
{
	while (!p->go)
	{
		usleep(1);
	}
	while (p->start < p->end)
	{
		std::cout << id << *(p->start) << " ";
		p->start++;
	}
	return 0;
}

int consumer(buffer_t* p,int id)
{
	while (!p->go)
	{
		usleep(1);
	}
	while (p->start < p->end)
	{
		usleep(1);
		std::cout << id << *(p->start) << " ";
		usleep(1);
		p->start++;
		usleep(1);
	}
	return 0;
}

int thread1(int* pa)
{
	int i;
	for (i = 0; i < 10000; i++)
	{
		(*pa)++;
	}
	return 0;
}

void test1()
{
	struct buffer_t buff;
	strcpy(buff.data,
			"abcdefghijklmnoprstuvwxyzabcdefghijklmnoprstuvwxyzabcdefghijklmnoprstuvwxyz");
	buff.start = buff.data;
	buff.end = buff.data + strlen(buff.data);
	buff.go = false;
	std::thread t1(consumer, &buff,1);
	std::thread t2(consumer, &buff,2);
	buff.go = true;
	t1.join();
	t2.join();
}

void test0()
{
	struct buffer_t buff;
	strcpy(buff.data,
			"abcdefghijklmnoprstuvwxyzabcdefghijklmnoprstuvwxyzabcdefghijklmnoprstuvwxyz");
	buff.start = buff.data;
	buff.end = buff.data + strlen(buff.data);
	buff.go = false;
	std::thread t1(fastconsumer, &buff,1);
	std::thread t2(fastconsumer, &buff,2);
	buff.go = true;
	t1.join();
	t2.join();
}

struct buffer2_t
{
	char data[100];
	char* start;
	char* end;
	bool go;
	std::mutex	mtx;
};

int good_consumer1(buffer2_t* p,int id)
{
	while (!p->go)
	{
		usleep(1);
	}
	while (p->start < p->end)
	{
		usleep(1);
		std::unique_lock<std::mutex> door(p->mtx);
		usleep(1);
		//door.lock();
		usleep(1);
		if (p->start < p->end)
		{
			usleep(1);
			std::cout << id << *(p->start) << " ";
			usleep(1);
			p->start++;
			usleep(1);
		}
		usleep(1);
		door.unlock();
		usleep(1);
	}
	return 0;
}

void test_good()
{
	struct buffer2_t buff;
	strcpy(buff.data,
			"abcdefghijklmnoprstuvwxyzabcdefghijklmnoprstuvwxyzabcdefghijklmnoprstuvwxyz");
	buff.start = buff.data;
	buff.end = buff.data + strlen(buff.data);
	buff.go = false;
	std::thread t1(good_consumer1, &buff,1);
	std::thread t2(good_consumer1, &buff,2);
	buff.go = true;
	t1.join();
	t2.join();
}

// waiting for data conditions until end
struct buffer3_t
{
	char data[100];
	char* start;
	char* end;
	char* max;		// no more data to process
	std::mutex	mtx;
	std::condition_variable cnd;
};

void realConsumer(buffer3_t* p,int id)
{
	do
	{
		usleep(1);
		std::unique_lock<std::mutex>  lock(p->mtx);	// close door
		p->cnd.wait(lock,[p]{return p->start != p->end || p->start >= p->max;});  // open door and wait
		if (p->start < p->end)
		{
			std::cout << id << *(p->start) << " ";
			p->start++;
		}
		if (p->start == p->end) p->cnd.notify_all();	// tell to everybody that we finish
	} while(p->start < p->max);
	std::cout << id << "-end";
}

void test_real()
{
	struct buffer3_t buff;
	strcpy(buff.data,
			"abcdefghijklmnoprstuvwxyzabcdefghijklmnoprstuvwxyzabcdefghijklmnoprstuvwxyz");
	buff.start = buff.data;
	buff.end = buff.data;
	buff.max = buff.data + strlen(buff.data);
	std::thread t1(realConsumer, &buff,1);
	std::thread t2(realConsumer, &buff,2);

	{
		std::unique_lock < std::mutex > lock(buff.mtx);	// close door
		buff.end += 2;
		buff.cnd.notify_one();
		buff.cnd.wait(lock, [&buff] {	return buff.start == buff.end;});		//open

		buff.end += 4;
		buff.cnd.notify_all();
		buff.cnd.wait(lock, [&buff] {	return buff.start == buff.end;});		//open

		buff.end += 30;
		buff.cnd.notify_all();
		buff.cnd.wait(lock, [&buff] {	return buff.start == buff.end;});

		buff.end = buff.max;	/// we do everything and finish
		buff.cnd.notify_all();
		buff.cnd.wait(lock, [&buff] {	return buff.start == buff.end;});
	} // the door is opne forever do not forget enclose in this

	t1.join();
	t2.join();
}

int main()
{
	std::cout << std::endl << "Test 0" << std::endl;
	test0();
	std::cout << std::endl << "Test 1" << std::endl;
	test1();
	std::cout << std::endl << "Test good" << std::endl;
	test_good();
	std::cout << std::endl << "Test real" << std::endl;
	test_real();
	std::cout << std::endl;
	return 0;

}
