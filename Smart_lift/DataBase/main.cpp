#include <iostream>
#include "DataBase.hpp"

int main(int argc, char** argv)
{
	setlocale(LC_ALL, "Russian");
	ServerDataBase sDB;
	sDB.start();
}