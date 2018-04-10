#pragma once
#include <string>

using std::string;

class User
{
public:
	User();
	~User();

	bool User::checkPassword(string pwd);
	void User::setPassword(string pwd);

private:
	string password;
};

