#include "build.hh"

#define CC "clang++"

int main(int argc, char **argv)
{
	Kitchen::Ingredients source_files{};

	const char *cflags[] = {};
	const char *cflags_debug[] = {};
	const char *ldflags[] = {};
	const char *libraries[] = {};

	source_files += "build.cpp";

	auto debug = Kitchen::Recipe("debug")
					 .compiler(CC)
					 .files(source_files)
					 .output("TEST_OUTPUT");

	Kitchen::Chef chef{};

	chef += debug;

	chef.cook();
}
