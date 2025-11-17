#include <iostream>
#include "dm_gcc_probe.cpp"

int main()
{
	std::cout << "Hello!" << __TIME__<< '\n';
	dm_gcc_CHECK("what?");
	dm_gcc_NOTICE("help");
	dm_gcc_ERROR("hello");
  dm_gcc_PROBE("Whoopsie!"); // provides return code
}
