//TO DO
//- Add a semaphore to tell other threads when a match has been found in the passwordCrack function

//Password Cracker by Zack Anderson (1602117)

//Libraries
#include <iostream>
#include <fstream>
#include <chrono>
#include <cstdlib>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "User.h"

//import from libraries
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::fstream;
using std::vector;
using std::thread;
using std::mutex;
using std::unique_lock;
using std::condition_variable;

typedef std::chrono::steady_clock the_clock;

//Global variables
mutex startPoint_mutex;
mutex cv_mutex;
condition_variable pwd_crack_cv;
bool pwd_cracked = false;
string cracked_password = "";

//function declarations
void setPwdlist(fstream &file, vector<string> &pwdList);
void passwordCrack(vector<string> &pwdList_short, User target);
void createThreads(int threads, vector<string> pwdList, User target);
void shorten_pwdList(vector<string> *pwdList, vector<string>::iterator *startPoint, User target, int threads, int size);

void main()
{
	string filepath;
	std::vector<string> pwdList;
	fstream file;
	User target;
	const int max_loops = 8;
	
	cout << "Input the filepath of the Dictionary list: " << endl;
	cin >> filepath;

	//file.open(filepath);

	//debug code
	string debugFilepath = "D:\\Uni Work\\Year 2\\Data Structures and Algorithms 2\\Password_Cracker\\common_passwords.txt";
	file.open(debugFilepath);
	target.setPassword("kittens");

	if (!file.is_open())					//checks to see if the file can be opened
	{
		cout << "Error opening file..." << endl;
		getchar();
		return;
	}

	setPwdlist(file, pwdList);				//Calls the function to set the wordlist for the password crack
	cout << "Attempting to crack password..." << endl;



	//the_clock::time_point start_total = the_clock::now();		//starts the clock
	for (int threads = 1; threads <= max_loops; threads++)
	{
		createThreads(threads, pwdList, target);
	}

	//::time_point end_total = the_clock::now();		//stops the clock
	//auto time_taken_total = duration_cast<milliseconds>(end_total - start_total).count();		//calculates the time taken
	
	//cout << endl << "Time taken to complete task: " << time_taken_total << "ms" << endl;
	cout << "Press any key to exit... " << endl;
	getchar();

	file.close();

	getchar();
	return;
}

void setPwdlist(fstream &file, vector<string> &pwdList)
{
	string line;

	cout << "Importing word list..." << endl;

	the_clock::time_point start = the_clock::now();
	while (getline(file, line))
	{
		pwdList.push_back(line);	//adds each line of the file to the vector
	}
	the_clock::time_point end = the_clock::now();
	auto time_taken = duration_cast<milliseconds>(end - start).count();
	
	cout << "Importing word list complete, took " << time_taken << "ms" << endl << endl;

	return;
}

void passwordCrack(vector<string> &pwdList_short, User target)
{
	for (vector<string>::iterator i = pwdList_short.begin(); i < pwdList_short.end(); i++)
	{
		if (target.checkPassword(*i) == true)			//If the password is found
		{
			//Uses a condition variable to tell the listener function if a password has been found
			unique_lock<mutex> lock(cv_mutex);
			cracked_password = *i;
			pwd_cracked = true;
			pwd_crack_cv.notify_all();
			return;
		}
		else if ((i + 1) == pwdList_short.end() && target.checkPassword(*i) == false)			//if the loop reaches the end of the list and checkPassword is still false
		{
			//cout << "Password not found. Perhaps try a larger word list?" << endl;
			return;
		}
	}

	pwdList_short.clear();						//empties the pwdList for the next thread to use

	return;
}

void shorten_pwdList(vector<string> *pwdList, vector<string>::iterator *startPoint, User target, int threads, int size)
{
	int listLength = size / threads;
	vector<string> pwdList_short;

	pwdList_short.clear();

	for (vector<string>::iterator i = *startPoint; i < (*startPoint + listLength); i++)
	{
		pwdList_short.push_back(*i);			//creates a shortened version of the wordlist for each thread to use (reduction)
	}

	{
		unique_lock<mutex> lock(startPoint_mutex);
		*startPoint = *startPoint + listLength;				//starts the next thread where the previous one left off
	}

	passwordCrack(pwdList_short, target);
}

void find_result()
{
	unique_lock<mutex> lock(cv_mutex);

	while (cracked_password == "")
	{
		pwd_crack_cv.wait(lock);
	}

	cout << "Password found! Password is: " << cracked_password << endl;
}

void createThreads(int threads, vector<string> pwdList, User target)
{
	vector<thread> threadpool;
	the_clock::time_point start = the_clock::now();
	vector<string>::iterator startPoint = pwdList.begin();
	int size = pwdList.size();

	thread result(find_result);

	for (int i = 0; i < threads; i++)
	{
		//threadpool.push_back(thread(bind(passwordCrack, &pwdList_short, target, &startPoint, listLength)));		//create the thread & the task for the thread (passes pwdList by reference)
		threadpool.push_back(thread(shorten_pwdList, &pwdList, &startPoint, target, threads, size));
	}

	for (int close_threads = 0; close_threads < threads; close_threads++)
	{
		// Wait for myThread to finish.
		threadpool[close_threads].join();

		// Now we just have the initial thread. So it's safe to exit.
	}

	if (pwd_cracked == false)
	{
		cout << "Password not found! Perhaps try a larger word list?" << endl;
	}

	the_clock::time_point end = the_clock::now();		//stops the clock
	auto time_taken = duration_cast<milliseconds>(end - start).count();		//calculates the time taken
	cout << "Time taken for " << threads << " number of threads: " << time_taken << "ms" << endl << endl;

	return;
}