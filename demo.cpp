#include "Luna.hpp"

#include <ctime>
#include <chrono>
#include <iostream>

uint32_t time_now()
{
	return std::time(nullptr);
}
uint32_t time_elapsed(uint32_t time)
{
	return std::time(nullptr) - time;
}
auto     time_unix_timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

int main(int argc, char* argv[])
{
	Luna luna;

	luna.LoadLibrary(LUNA_LIBRARY_BASE);

	if (auto time = luna.CreateTable())
	{
		time.SetField("now",            &time_now);
		time.SetField("elapsed",        &time_elapsed);
		time.SetField("unix_timestamp", time_unix_timestamp);

		luna.SetGlobal("time", time);
	}

	if (auto time = luna.CreateLibrary())
	{
		time.SetField("now",            &time_now);
		time.SetField("elapsed",        &time_elapsed);
		time.SetField("unix_timestamp", time_unix_timestamp);

		luna.LoadLibrary(time, "libtime", true);
	}

	luna.SetGlobal("time_now",       &time_now);
	luna.SetGlobal("time_elapsed",   &time_elapsed);
	luna.SetGlobal("unix_timestamp", time_unix_timestamp);

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
