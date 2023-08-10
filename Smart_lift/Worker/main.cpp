#include <iostream>
#include "WorkerServer.hpp"

int main(int argc, char** argv)
{
	setlocale(LC_ALL, ".UTF-8");
	ServerWorker sW("");
	sW.run();
}