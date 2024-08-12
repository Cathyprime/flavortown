#include "build.hh"

#define CC "clang++"

int main(int argc, char** argv)
{
	Kitchen::Ingredients source_files{};
	Kitchen::Ingredients fib_files1{};
	Kitchen::Ingredients fib_files2{};
	Kitchen::Ingredients fib_files3{};

	Kitchen::Ingredients cflags{};
	cflags += "-Wall";
	cflags += "-Werror";

	Kitchen::Ingredients fib_flags{};
	fib_flags += "-c";

	source_files += "build.cpp";
	fib_files1 += "fibonacci_src/fibonacci1.cpp";
	fib_files2 += "fibonacci_src/fibonacci2.cpp";
	fib_files3 += "fibonacci_src/fibonacci3.cpp";

	auto debug = Kitchen::CppRecipe("debug").compiler(CC).files(source_files).output("TEST_OUTPUT");

	auto release = Kitchen::CppRecipe("release")
					   .compiler(CC)
					   .files(source_files)
					   .optimization(Kitchen::Heat::O2)
					   .cpp_version("c++23")
					   .output("release")
					   .cflags(cflags);

	auto fib1 =
		Kitchen::CppRecipe("fib1").compiler(CC).files(fib_files1).output("build_dir/fibonacci1.o").cflags(fib_flags);
	auto fib2 =
		Kitchen::CppRecipe("fib2").compiler(CC).files(fib_files2).output("build_dir/fibonacci2.o").cflags(fib_flags);
	auto fib3 =
		Kitchen::CppRecipe("fib3").compiler(CC).files(fib_files3).output("build_dir/fibonacci3.o").cflags(fib_flags);

	auto combined_files = Kitchen::Ingredients{};
	combined_files += "build_dir/fibonacci1.o";
	combined_files += "build_dir/fibonacci2.o";
	combined_files += "build_dir/fibonacci3.o";
	combined_files += "fibonacci_src/fibonacci_combined.cpp";

	auto combined_fib = Kitchen::CppRecipe("combined_fib").compiler(CC).files(combined_files).output("bin/fib");

	Kitchen::Chef chef{};
	Kitchen::LineCook line_cook{};

	chef += &debug;
	chef += &release;
	chef += &combined_fib;

	line_cook += &fib1;
	line_cook += &fib2;
	line_cook += &fib3;

	if (argc > 1 && argv[1] == std::string("combined_fib")) {
		line_cook.cook();
		chef.dessert("combined_fib");
	} else if (argc > 1 && argv[1] == std::string("release"))
		chef.dessert("release");
	else
		chef.dessert();
}
