#include <iostream>
#include "WorkerServer.hpp"

int main(int argc, char** argv)
{
	setlocale(LC_ALL, "ru_RU.UTF-8");
	ServerWorker sW("");
	sW.run();
}