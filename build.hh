#pragma once

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <numeric>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

#ifndef CC
#define CC "clang++"
#endif

#ifdef _WIN32
#define WIN_LEAN_AND_MEAN
#define GO_REBUILD_YOURSELF(argc, argv)                                                                                \
	do {                                                                                                               \
	} while (0)
#else
#ifdef GUY_FIERI
#define GO_REBUILD_YOURSELF(argc, argv)                                                                                \
	do {                                                                                                               \
		std::filesystem::path source_file = std::filesystem::path(__FILE__);                                           \
		std::string executable_name = Kitchen::get_executable_name(source_file.filename().string());                   \
		std::filesystem::path executable = std::filesystem::path(executable_name);                                     \
                                                                                                                       \
		if (!std::filesystem::exists(executable) ||                                                                    \
			std::filesystem::last_write_time(source_file) > std::filesystem::last_write_time(executable)) {            \
			std::cout << "[INFO]: Rebuilding " << executable << "...\n";                                               \
			std::vector<std::string> command = {CC, "-DGUY_FIERI", "-Oz", "-o", executable_name, __FILE__};            \
			std::string cmd_str =                                                                                      \
				std::string(CC) + " -DGUY_FIERI -Oz -o " + executable_name + " " + std::string(__FILE__);              \
			std::system(cmd_str.c_str());                                                                              \
                                                                                                                       \
			std::vector<const char*> new_argv(argc);                                                                   \
			new_argv[0] = ("./" + executable_name).c_str();                                                            \
			for (int i = 1; i < argc; ++i) {                                                                           \
				new_argv[i] = argv[i];                                                                                 \
			}                                                                                                          \
			new_argv.push_back(nullptr);                                                                               \
			execv(("./" + executable_name).c_str(), const_cast<char* const*>(new_argv.data()));                        \
		}                                                                                                              \
	} while (0)
#else
#define GO_REBUILD_YOURSELF(argc, argv)                                                                                \
	do {                                                                                                               \
		std::filesystem::path source_file = std::filesystem::path(__FILE__);                                           \
		std::string executable_name = Kitchen::get_executable_name(source_file.filename().string());                   \
		std::filesystem::path executable = std::filesystem::path(executable_name);                                     \
                                                                                                                       \
		std::cout << "[INFO]: Rebuilding " << executable << " with optimizations...\n";                                \
		std::vector<std::string> command = {CC, "-DGUY_FIERI", "-Oz", "-o", executable_name, __FILE__};                \
		std::string cmd_str =                                                                                          \
			std::string(CC) + " -DGUY_FIERI -Oz -o " + executable_name + " " + std::string(__FILE__);                  \
		std::system(cmd_str.c_str());                                                                                  \
                                                                                                                       \
		std::vector<const char*> new_argv(argc);                                                                       \
		new_argv[0] = ("./" + executable_name).c_str();                                                                \
		for (int i = 1; i < argc; ++i) {                                                                               \
			new_argv[i] = argv[i];                                                                                     \
		}                                                                                                              \
		new_argv.push_back(nullptr);                                                                                   \
		execv(("./" + executable_name).c_str(), const_cast<char* const*>(new_argv.data()));                            \
	} while (0)
#endif // GUY_FIERI
#endif // _WIN32

#define INGREDIENTS_SETTER(method_name, member_variable)                                                               \
	inline CppRecipe& CppRecipe::method_name(Ingredients& value)                                                       \
	{                                                                                                                  \
		method_name(std::move(value));                                                                                 \
		return *this;                                                                                                  \
	}                                                                                                                  \
	inline CppRecipe& CppRecipe::method_name(Ingredients&& value)                                                      \
	{                                                                                                                  \
		member_variable = value.get_ingredients();                                                                     \
		return *this;                                                                                                  \
	}

#define SETTER(class, method_name, type, member_variable)                                                              \
	inline class& class ::method_name(type value)                                                                      \
	{                                                                                                                  \
		member_variable = value;                                                                                       \
		return *this;                                                                                                  \
	}

namespace Kitchen
{

int start_job_sync(const std::vector<std::string>& command);
void print_command(const std::vector<std::string>& command);

enum class Heat;

class Recipe
{
  public:
	virtual ~Recipe(){};
	virtual std::vector<std::string> get_command() = 0;
	virtual const std::string& get_name() = 0;
	virtual bool rebuild_needed() = 0;
};

class Chef;
class Ingredients;

class Ingredients
{
  private:
	std::vector<std::string> m_Files;
	std::string m_Prefix;

  public:
	Ingredients() = default;
	Ingredients(Ingredients&& rhs) = default;
	Ingredients(const Ingredients& rhs) = default;
	Ingredients& operator=(const Ingredients& rhs) = default;
	Ingredients& prefix(const std::string& prefix);
	Ingredients& add_ingredients(const std::string& file);
	void operator+=(const std::string& file);

	std::vector<std::string> get_ingredients();
};

class CppRecipe : public Recipe
{
  private:
	bool m_Cache;
	std::string m_Name;
	std::filesystem::path m_Output;
	std::string m_Version;
	std::string m_Compiler;
	std::string m_Optimization_level;
	std::vector<std::string> m_Libs;
	std::vector<std::string> m_Cflags;
	std::vector<std::string> m_Ldflags;
	std::optional<Ingredients> m_Files;

  public:
	CppRecipe(std::string name)
		: m_Cache(false), m_Name(name), m_Output(), m_Version(), m_Compiler(), m_Optimization_level(), m_Libs(),
		  m_Cflags(), m_Ldflags(), m_Files(std::nullopt){};

	const std::string& get_name() override;

	CppRecipe() = default;
	CppRecipe(const CppRecipe& rhs) = default;
	CppRecipe(CppRecipe&& rhs) = default;

	CppRecipe& files(const Ingredients& files);
	CppRecipe& output(const std::string& name);
	CppRecipe& optimization(const Heat& level);
	CppRecipe& optimization(std::string&& level);
	CppRecipe& compiler(const std::string& compiler);
	CppRecipe& cpp_version(const std::string& version);
	CppRecipe& cflags(Ingredients& cflags);
	CppRecipe& cache();
	CppRecipe& cache(bool cache);
	CppRecipe& cflags(Ingredients&& cflags);
	CppRecipe& ldflags(Ingredients& ldflags);
	CppRecipe& ldflags(Ingredients&& ldflags);
	CppRecipe& libraries(Ingredients& libraries);
	CppRecipe& libraries(Ingredients&& libraries);

	bool rebuild_needed() override;
	std::vector<std::string> get_command() override;
};

class Chef
{
  private:
	size_t m_Defaut_recipe_index;
	std::vector<Recipe*> m_Recipes;
	static int cook(Recipe* recipe);
	friend class LineCook;

  public:
	Chef& default_recipe(Recipe* recipe);
	Chef& learn_recipe(Recipe* recipe);
	int cook();
	int cook(const std::string& name);
	void dessert();
	void dessert(const std::string& name);

	void operator+=(Recipe* recipe);
};

class LineCook : public Chef
{
  private:
	std::vector<Recipe*> m_Recipes;
	static int cook(Recipe* recipe);

  public:
	LineCook& learn_recipe(Recipe* recipe);
	int cook();
	int cook(const std::string& name);
	void dessert();
	void dessert(const std::string& name);
	void operator+=(Recipe* recipe);
};

enum class Heat { O0, O1, O2, O3, Ofast, Os, Oz, Og };

inline int start_job_sync(const std::vector<std::string>& command)
{
	int result = std::system(
		std::accumulate(std::next(command.begin()), command.end(), command[0], [](std::string a, std::string b) {
			return a + " " + b;
		}).c_str());

#ifndef _WIN32
	if (WIFEXITED(result)) {
		int exit_status = WEXITSTATUS(result);
		return exit_status;
	}
#endif // _WIN32
	return result;
}

inline void print_command(const std::vector<std::string>& command)
{
	std::stringstream ss{};
	ss << "[COMMAND]:";
	for (const auto& c : command)
		ss << " " << c;
	ss << std::endl;
	std::cout << ss.str();
}

inline void Ingredients::operator+=(const std::string& file) { (void)add_ingredients(file); }

SETTER(Ingredients, prefix, const std::string&, m_Prefix);

inline Ingredients& Ingredients::add_ingredients(const std ::string& value)
{
	m_Files.push_back(value);
	return *this;
};

inline std::vector<std::string> Ingredients::get_ingredients()
{
	std::vector<std::string> ret;

	for (auto file_str : m_Files) {
		auto file = std::filesystem::path(file_str).make_preferred().string();
		if (m_Prefix != "" && file.find(m_Prefix) == 0)
			ret.push_back(file.substr(m_Prefix.length()));
		else
			ret.push_back(file);
	}

	if (!m_Prefix.empty())
		for (size_t i = 0; i < m_Files.size(); ++i)
			ret[i] = m_Prefix + ret[i];

	return ret;
}

inline CppRecipe& CppRecipe::optimization(const Heat& level)
{
	switch (level) {
	case Heat::O0: m_Optimization_level = "-O0"; break;
	case Heat::O1: m_Optimization_level = "-O1"; break;
	case Heat::O2: m_Optimization_level = "-O2"; break;
	case Heat::O3: m_Optimization_level = "-O3"; break;
	case Heat::Ofast: m_Optimization_level = "-Ofast"; break;
	case Heat::Os: m_Optimization_level = "-Os"; break;
	case Heat::Oz: m_Optimization_level = "-Oz"; break;
	case Heat::Og: m_Optimization_level = "-Og"; break;
	}
	return *this;
}

inline CppRecipe& CppRecipe::optimization(std::string&& level)
{
	if (level.find("-") != 0) level = "-" + level;
	m_Optimization_level = level;
	return *this;
}

INGREDIENTS_SETTER(cflags, m_Cflags)
INGREDIENTS_SETTER(ldflags, m_Ldflags)
INGREDIENTS_SETTER(libraries, m_Libs)
SETTER(CppRecipe, cache, bool, m_Cache)
SETTER(CppRecipe, files, const Ingredients&, m_Files)
SETTER(CppRecipe, compiler, const std::string&, m_Compiler)
SETTER(CppRecipe, cpp_version, const std::string&, m_Version)

inline CppRecipe& CppRecipe::cache()
{
	m_Cache = !m_Cache;
	return *this;
}

inline CppRecipe& CppRecipe::output(const std::string& value)
{
	m_Output = value;
	m_Output = m_Output.make_preferred();
	return *this;
}

inline std::vector<std::string> CppRecipe::get_command()
{
	assert((m_Compiler != "" && "ERROR: compiler is REQUIRED to COMPILE...\n"));
	assert((m_Files.has_value() && "ERROR: you need to provide files to compile"));

	std::vector<std::string> command = {m_Compiler};

	for (const auto& cflag : m_Cflags)
		command.push_back(cflag);

	if (m_Version != "") command.push_back("-std=" + m_Version);

	if (m_Optimization_level != "") command.push_back(m_Optimization_level);

	for (const auto& library : m_Libs)
		command.push_back("-l" + library);

	if (m_Output != "") {
		command.push_back("-o");
		command.push_back(m_Output.string());
		auto output = m_Output;
		auto path = output.remove_filename();
		if (!std::filesystem::exists(path) && path.has_relative_path()) std::filesystem::create_directories(path);
	}

	for (std::filesystem::path file : m_Files->get_ingredients())
		command.push_back(file.make_preferred());

	for (const auto& ldflag : m_Ldflags)
		command.push_back(ldflag);

	return command;
}

inline const std::string& CppRecipe::get_name() { return m_Name; }

inline bool CppRecipe::rebuild_needed()
{
	if (!m_Cache) return true;

	bool should_rebuild = false;
	std::filesystem::path output = m_Output.make_preferred().string();

	for (auto& input_file_str : m_Files->get_ingredients()) {
		std::filesystem::path input_file = input_file_str;
		auto in_file_mod_time = std::filesystem::last_write_time(input_file);
		if (!std::filesystem::exists(output) || std::filesystem::last_write_time(output) < in_file_mod_time) {
			should_rebuild = true;
			break;
		}
	}

	return should_rebuild;
}

inline Chef& Chef::default_recipe(Recipe* recipe)
{
	m_Defaut_recipe_index = m_Recipes.size();
	m_Recipes.push_back(recipe);
	return *this;
}

inline Chef& Chef::learn_recipe(Recipe* value)
{
	m_Recipes.push_back(value);
	return *this;
}

inline void Chef::operator+=(Recipe* recipe)
{
	if (m_Recipes.size() == 0)
		default_recipe(recipe);
	else
		learn_recipe(recipe);
}

inline int Chef::cook(Recipe* recipe)
{
	int status = 0;

	if (recipe->rebuild_needed()) {
		auto command = recipe->get_command();
		print_command(command);
		status = start_job_sync(std::move(command));
	}
	return status;
}

inline int Chef::cook() { return cook(m_Recipes[m_Defaut_recipe_index]); }

inline void Chef::dessert() { std::exit(cook()); }

inline int Chef::cook(const std::string& recipe_name)
{
	assert((recipe_name != "" && "recipe_name cannot be empty"));

	auto recipe = std::find_if(m_Recipes.begin(), m_Recipes.end(),
							   [&recipe_name](auto& recipe) { return recipe->get_name() == recipe_name; });

	return cook(recipe[0]);
}

inline void Chef::dessert(const std::string& recipe_name) { std::exit(cook(recipe_name)); }

inline LineCook& LineCook::learn_recipe(Recipe* recipe)
{
	m_Recipes.push_back(recipe);
	return *this;
}

inline void LineCook::operator+=(Recipe* recipe) { learn_recipe(std::move(recipe)); }

inline int LineCook::cook(const std::string& name)
{
	(void)name;
	return cook();
}

inline void LineCook::dessert(const std::string& name)
{
	(void)name;
	std::exit(cook());
}

inline void LineCook::dessert() { std::exit(cook()); }

inline int LineCook::cook()
{
	std::vector<std::thread> threads;
	std::atomic<int> error(0);

	for (auto& recipe : m_Recipes) {
		if (recipe->rebuild_needed()) {
			auto command = recipe->get_command();
			threads.push_back(std::thread([command, &error]() {
				if (error.load() != 0) return;

				int status = 0;
				print_command(command);
				status = start_job_sync(command);

				if (status != 0) {
					std::stringstream msg;
					msg << "Thread exited with error status: " << status << std::endl;
					std::cerr << msg.str();
					error.store(status);
				}
			}));
		}
	}

	for (auto& thread : threads)
		thread.join();

	return error.load();
}
// {
// 	std::vector<std::thread> threads;
// 	std::atomic<int> error(0);
//
// 	for (auto& recipe : m_Recipes) {
// 		auto command = recipe->get_command();
// 		threads.push_back(std::thread([recipe, command, &error]() {
// 			if (error.load() != 0) return;
//
// 			int status = Chef::cook(recipe);
// 			if (status != 0) {
// 				std::stringstream msg;
// 				msg << "Thread exited with error status: " << status << std::endl;
// 				std::cerr << msg.str();
// 				error.store(status);
// 			}
// 		}));
// 	}
//
// 	for (auto& thread : threads)
// 		thread.join();
//
// 	int status = error.load();
// 	return status;
// }

inline std::string get_executable_name(const std::string& source_file)
{
	std::string executable_name = source_file;
	auto dot_pos = executable_name.find_last_of('.');
	if (dot_pos != std::string::npos) {
		executable_name = executable_name.substr(0, dot_pos); // Remove the extension
	}
	return executable_name;
}

} // namespace Kitchen
