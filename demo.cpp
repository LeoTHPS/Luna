#include "Luna.hpp"

#include <iostream>

int a = 0;
int b = 0;

int do_the_thing(int a, int b)
{
	::a = a;
	::b = b;

	return a + b;
}
int run_the_thing(LunaFunction<int(int a, int b)> callback)
{
	return callback(a, b);
}

int main(int argc, char* argv[])
{
	Luna luna;

	luna.SetGlobal("do_the_thing", do_the_thing);
	luna.SetGlobal("run_the_thing", run_the_thing);
	luna.LoadLibrary(LUNA_LIBRARY_BASE);

	try
	{
		luna.Run("print(do_the_thing(1, 1));");
		luna.Run("print(run_the_thing(function(a, b) return a + b; end));");
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}
