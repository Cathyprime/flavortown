#include "build.hh"

#define CC "clang++"

int main(int argc, char** argv)
{
	Kitchen::Ingredients source_files{};

	const char* cflags[] = {"-Wall"};

	source_files += "build.cpp";

	auto debug = Kitchen::Recipe("debug").compiler(CC).files(source_files).output("TEST_OUTPUT");

	auto release = Kitchen::Recipe("release")
					   .compiler(CC)
					   .files(source_files)
					   .optimization(Kitchen::Heat::O2)
					   .output("release")
					   .cflags(cflags, sizeof(cflags) / sizeof(cflags[0]));

	Kitchen::Chef chef{};

	chef += debug;
	chef += release;

	if (argc > 1 && argv[1] == std::string("release"))
		chef.cook("release");
	else
		chef.cook();

}
