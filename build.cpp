#include "build.hh"

int main(int argc, char** argv)
{
	GO_REBUILD_YOURSELF(argc, argv);

	Kitchen::Ingredients source_files{};
	Kitchen::Ingredients fib_files1{};
	Kitchen::Ingredients fib_files2{};
	Kitchen::Ingredients fib_files3{};

	Kitchen::Ingredients cflags{};
	cflags += "-Wall";
	cflags += "-Werror";

	Kitchen::Ingredients fib_flags = cflags;
	fib_flags += "-c";

	auto debug_flags = cflags;
	debug_flags += "-g";

	source_files += __FILE_NAME__;
	fib_files1 += "fibonacci_src/fibonacci1.cpp";
	fib_files2 += "fibonacci_src/fibonacci2.cpp";
	fib_files3 += "fibonacci_src/fibonacci3.cpp";

	auto debug = Kitchen::CompilerRecipe("debug")
					 .compiler(CC)
					 .files(source_files)
					 .output("TEST_OUTPUT")
					 .push(debug_flags.get_ingredients());

	auto release = Kitchen::CompilerRecipe("release")
					   .compiler(CC)
					   .files(source_files)
					   .optimization(Kitchen::Heat::O2)
					   .std_version("c++23")
					   .output("release")
					   .push(cflags.get_ingredients());

	auto fib1 = Kitchen::CompilerRecipe("fib1")
					.compiler(CC)
					.cache()
					.files(fib_files1)
					.output("build_dir/fibonacci1.o")
					.push(fib_flags.get_ingredients());

	auto fib2 = Kitchen::CompilerRecipe("fib2")
					.compiler(CC)
					.cache()
					.files(fib_files2)
					.output("build_dir/fibonacci2.o")
					.push(fib_flags.get_ingredients());

	auto fib3 = Kitchen::CompilerRecipe("fib3")
					.compiler(CC)
					.cache()
					.files(fib_files3)
					.output("build_dir/fibonacci3.o")
					.push(fib_flags.get_ingredients());

	auto combined_files = Kitchen::Ingredients{};
	combined_files += "build_dir/fibonacci1.o";
	combined_files += "build_dir/fibonacci2.o";
	combined_files += "build_dir/fibonacci3.o";
	combined_files += "fibonacci_src/fibonacci_combined.cpp";

	auto combined_fib
		= Kitchen::CompilerRecipe("combined_fib").compiler(CC).cache().files(combined_files).output("bin/fib");

	Kitchen::LineCook line_cook{};

	line_cook += &fib1;
	line_cook += &fib2;
	line_cook += &fib3;

	if (argc > 1 && argv[1] == std::string("combined_fib")) {
		int status = line_cook.cook();
		if (status != 0) std::exit(status);
		std::exit(Kitchen::cook(&combined_fib));
	} else if (argc > 1 && argv[1] == std::string("release"))
		std::exit(Kitchen::cook(&release));
	else
		std::exit(Kitchen::cook(&debug));
}
