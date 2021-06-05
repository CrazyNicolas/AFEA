#include "Internal_1.h"
#include "Internal_2.h"
#include <string>
#pragma warning(disable : 4996)

/* Interface: ./Internal data_path algorithm[Internal_1 / Internal_2] */
Internal_1 solver1;
Internal_2 solver2;
int main(int argc, char* argv[])
{
	char* data_path = argv[1];
	int* res;
	std::string algorithm = argv[2];
	if (algorithm == "Internal_1")
	{
		solver1.solve(data_path);
	}
	else
	{
		solver2.solve(data_path);
	}
}
