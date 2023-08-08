#include <iostream>
#include "WorkerServer.hpp"

int main(int argc, char** argv)
{
	setlocale(LC_ALL, "Russian");
	ServerWorker sW("");
	sW.run();
}