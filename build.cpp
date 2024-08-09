#include "build.hh"

#define CC "clang++"

int main(int argc, char** argv)
{
	Kitchen::Ingredients source_files{};

	Kitchen::Ingredients cflags{};
	cflags.prefix("-W");
	cflags += "all";
	cflags += "error";

	source_files += "build.cpp";

	auto debug = Kitchen::CppRecipe("debug").compiler(CC).files(source_files).output("TEST_OUTPUT");

	auto release = Kitchen::CppRecipe("release")
					   .compiler(CC)
					   .files(source_files)
					   .optimization(Kitchen::Heat::O2)
					   .output("release")
					   .cflags(cflags);

	Kitchen::Chef chef{};

	chef += debug;
	chef += release;

	if (argc > 1 && argv[1] == std::string("release"))
		chef.cook("release");
	else
		chef.cook();
}
