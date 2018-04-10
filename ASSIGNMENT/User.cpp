#include "User.h"



User::User()
{
}


User::~User()
{
}

void User::setPassword(string pwd)
{
	password = pwd;		//sets the password of the user
}

bool User::checkPassword(string pwd)
{
	if (pwd == password)
	{
		return true;
	}
	else
	{
		return false;
	}
}
