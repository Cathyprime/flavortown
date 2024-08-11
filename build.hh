#pragma once
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <optional>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

// internal use only
#define TODO1(str) std::cerr << __FILE__ << ":" << __LINE__ << ":TODO -> " << str << std::endl
#define TODO0() std::cerr << __FILE__ << ":" << __LINE__ << ":TODO" << std::endl
#define GET_MACRO(_0, NAME, ...) NAME

#define TODO(...) GET_MACRO(__VA_ARGS__ __VA_OPT__(, ) TODO1, TODO0)(__VA_ARGS__)

namespace Kitchen
{

int start_job_sync(std::vector<std::string> command);

enum class Heat;

class Chef;
class CppRecipe;
class Ingredients;

class Ingredients
{
  private:
	std::vector<std::string> m_Files;
	std::string m_Prefix;

  public:
	Ingredients& prefix(const std::string& prefix);
	Ingredients& add_ingredients(const std::string& file);
	// Ingredients& glob_source(const std::string& glob);
	void operator+=(const std::string& file);

	std::vector<std::string> get_ingredients();
};

class CppRecipe
{
  private:
	std::string m_Name;
	std::string m_Output;
	std::string m_Version;
	std::string m_Compiler;
	std::string m_Optimization_level;
	std::vector<std::string> m_Libs;
	std::vector<std::string> m_Cflags;
	std::vector<std::string> m_Ldflags;
	std::optional<Ingredients> m_Files;

  public:
	CppRecipe(std::string name)
		: m_Name(name), m_Output(), m_Version(), m_Compiler(), m_Optimization_level(), m_Libs(), m_Cflags(), m_Ldflags(),
		  m_Files(std::nullopt){};

	const std::string& get_name();

	CppRecipe& files(const Ingredients& files);
	CppRecipe& output(const std::string& name);
	CppRecipe& optimization(const Heat& level);
	CppRecipe& optimization(std::string&& level);
	CppRecipe& compiler(const std::string& compiler);
	CppRecipe& cpp_version(const std::string& version);
	CppRecipe& cflags(Ingredients& cflags);
	CppRecipe& cflags(Ingredients&& cflags);
	CppRecipe& ldflags(Ingredients& ldflags);
	CppRecipe& ldflags(Ingredients&& ldflags);
	CppRecipe& libraries(Ingredients& libraries);
	CppRecipe& libraries(Ingredients&& libraries);

	std::vector<std::string> get_command();
};

class Chef
{
  private:
	size_t m_Defaut_recipe_index;
	std::vector<CppRecipe> m_Recipes;

  public:
	Chef& default_recipe(const CppRecipe& recipe);
	Chef& learn_recipe(const CppRecipe& recipe);
	void cook();
	void cook(const std::string& name);

	void operator+=(const CppRecipe& recipe);
};

#ifndef GUY_FIERI_BUILD_SYSTEM
#define GUY_FIERI_BUILD_SYSTEM
enum class Heat { O0, O1, O2, O3, Ofast, Os, Oz, Og };

inline int start_job_sync(std::vector<std::string> command)
{
	std::vector<char*> c_args{};
	c_args.reserve(command.size());

	std::cout << "[COMMAND]:";
	for (auto& arg : command) {
		std::cout << " " << arg;
		c_args.push_back(const_cast<char* const>(arg.c_str()));
	}
	std::cout << std::endl;
	c_args.push_back(nullptr);

	pid_t pid = fork();
	if (pid == 0) {
		execvp(command[0].c_str(), c_args.data());
		std::cerr << "execvp failed to start: " << strerror(errno) << std::endl;
		exit(errno);
	} else if (pid > 0) {
		int status;

		if (waitpid(pid, &status, 0) == -1) {
			std::cerr << "waitpid failed: " << strerror(errno) << std::endl;
			exit(errno);
		}

		if (WIFEXITED(status)) {
			return WEXITSTATUS(status);
		} else {
			std::cerr << "Child process did not exit normally" << std::endl;
			return status;
		}
	} else {
		std::cerr << "Failed to fork: " << strerror(errno) << std::endl;
		exit(errno);
	}
}

inline void Ingredients::operator+=(const std::string& file) { (void)add_ingredients(file); }

inline Ingredients& Ingredients::prefix(const std::string& prefix)
{
	m_Prefix = prefix;
	return *this;
}

inline Ingredients& Ingredients::add_ingredients(const std::string& file)
{
	this->m_Files.push_back(file);
	return *this;
}

inline std::vector<std::string> Ingredients::get_ingredients()
{
	std::vector<std::string> ret;

	for (auto file : m_Files)
		if (m_Prefix != "" && file.find(m_Prefix) == 0)
			ret.push_back(file.substr(m_Prefix.length()));
		else
			ret.push_back(file);

	for (size_t i = 0; i < m_Files.size(); ++i)
		ret[i] = m_Prefix + ret[i];

	return ret;
}

inline CppRecipe& CppRecipe::compiler(const std::string& compiler)
{
	m_Compiler = compiler;
	return *this;
}

inline CppRecipe& CppRecipe::cpp_version(const std::string& version)
{
	m_Version = version;
	return *this;
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
	if (level.find("-") != 0)
		level = "-" + level;

	m_Optimization_level = level;
	return *this;
}

inline CppRecipe& CppRecipe::cflags(Ingredients& cflags)
{
	m_Cflags = cflags.get_ingredients();
	return *this;
}

inline CppRecipe& CppRecipe::cflags(Ingredients&& cflags)
{
	m_Cflags = cflags.get_ingredients();
	return *this;
}

inline CppRecipe& CppRecipe::ldflags(Ingredients& flags)
{
	m_Ldflags = flags.get_ingredients();
	return *this;
}

inline CppRecipe& CppRecipe::ldflags(Ingredients&& flags)
{
	m_Ldflags = flags.get_ingredients();
	return *this;
}

inline CppRecipe& CppRecipe::libraries(Ingredients& libraries)
{
	m_Libs = libraries.get_ingredients();
	return *this;
}

inline CppRecipe& CppRecipe::libraries(Ingredients&& libraries)
{
	m_Libs = libraries.get_ingredients();
	return *this;
}

inline CppRecipe& CppRecipe::output(const std::string& name)
{
	m_Output = name;
	return *this;
}

inline std::vector<std::string> CppRecipe::get_command()
{
	assert((m_Compiler != "" && "ERROR: compiler is REQUIRED to COMPILE...\n"));
	assert((m_Files.has_value() && "ERROR: you need to provide files to compile"));

	std::vector<std::string> command = {m_Compiler};

	for (const auto& cflag : m_Cflags)
		command.push_back(cflag);

	if (m_Version != "")
		command.push_back("-std="+m_Version);

	if (m_Optimization_level != "")
		command.push_back(m_Optimization_level);

	for (const auto& library : m_Libs)
		command.push_back("-l" + library);

	if (m_Output != "") {
		command.push_back("-o");
		command.push_back(m_Output);
	}

	for (const auto& file : m_Files->get_ingredients())
		command.push_back(file);

	for (const auto& ldflag : m_Ldflags)
		command.push_back(ldflag);

	return command;
}

inline CppRecipe& CppRecipe::files(const Ingredients& files)
{
	m_Files = files;
	return *this;
}

inline const std::string& CppRecipe::get_name() { return m_Name; }

inline Chef& Chef::default_recipe(const CppRecipe& recipe)
{
	m_Defaut_recipe_index = m_Recipes.size();
	m_Recipes.push_back(recipe);
	return *this;
}

inline Chef& Chef::learn_recipe(const CppRecipe& recipe)
{
	m_Recipes.push_back(recipe);
	return *this;
}

inline void Chef::operator+=(const CppRecipe& recipe)
{
	if (m_Recipes.size() == 0)
		default_recipe(recipe);
	else
		learn_recipe(recipe);
}

inline void Chef::cook() { exit(start_job_sync(m_Recipes[m_Defaut_recipe_index].get_command())); }

inline void Chef::cook(const std::string& recipe_name)
{
	assert((recipe_name != "" && "recipe_name cannot be empty"));

	auto recipe = std::find_if(m_Recipes.begin(), m_Recipes.end(),
							   [recipe_name](auto& recipe) { return recipe.get_name() == recipe_name; });

	start_job_sync(recipe[0].get_command());
}

#endif // GUY_FIERI_BUILD_SYSTEM
} // namespace Kitchen
