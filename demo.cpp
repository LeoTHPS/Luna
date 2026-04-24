#include "Luna.hpp"

#include <ctime>
#include <iostream>

uint32_t time_now()
{
	return std::time(nullptr);
}
uint32_t time_elapsed(uint32_t time)
{
	return std::time(nullptr) - time;
}

int main(int argc, char* argv[])
{
	Luna luna;

	luna.LoadLibrary(LUNA_LIBRARY_BASE);

	if (auto time = luna.CreateTable())
	{
		time.SetField("now",     &time_now);
		time.SetField("elapsed", &time_elapsed);

		luna.SetGlobal("time", time);
	}

	luna.SetGlobal("time_now",     &time_now);
	luna.SetGlobal("time_elapsed", &time_elapsed);

	try
	{
		luna.RunFile("demo.lua");
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}
