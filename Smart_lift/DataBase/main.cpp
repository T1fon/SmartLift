#include <iostream>
#include "DataBase.hpp"

int main(int argc, char** argv)
{
	setlocale(LC_ALL, "ru_RU.UTF-8");
	ServerDataBase sDB;
	sDB.start();
}