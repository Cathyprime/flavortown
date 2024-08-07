// definition
#pragma once
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

#define TODO1(str) std::cerr << __FILE__ << ":" << __LINE__ << ":TODO -> " << str << std::endl
#define TODO0() std::cerr << __FILE__ << ":" << __LINE__ << ":TODO" << std::endl

#define GET_MACRO(_0, NAME, ...) NAME
#define TODO(...) GET_MACRO(__VA_ARGS__ __VA_OPT__(, ) TODO1, TODO0)(__VA_ARGS__)

namespace Kitchen
{
struct Command {
	std::string command;
	std::vector<char*> args;
	std::vector<std::string> arg_strings;
};

void start_job(Command command);

enum class Heat;

class Chef;
class Recipe;
class Ingredients;

class Ingredients
{
  private:
	std::vector<std::string> m_Files;
	std::string m_Prefix;

  public:
	Ingredients &prefix(const std::string &prefix);
	Ingredients &add_source(const std::string &file);
	Ingredients &glob_source(const std::string &glob);
	void operator+=(const std::string &file);
	void print_files();

	std::vector<std::string> get_files();
};

class Recipe
{
  private:
	std::string m_Compiler;
	std::string m_Optimization_level;
	std::string m_Output;
	std::vector<std::string> m_Cflags;
	std::vector<std::string> m_Ldflags;
	std::vector<std::string> m_Libs;
	std::optional<Ingredients> m_Files;
	bool m_Debug;
	std::string m_Name;

  public:
	Recipe(std::string name)
		: m_Compiler(), m_Optimization_level(), m_Cflags(), m_Ldflags(), m_Libs(), m_Files(std::nullopt), m_Debug(),
		  m_Name(name)
	{
	}

	Recipe &compiler(const std::string &compiler);
	Recipe &optimization(const Heat &level);
	Recipe &optimization(const std::string &level);
	Recipe &output(const std::string &name);
	Recipe &cflags(const char **flags, size_t flag_count);
	Recipe &ldflags(const char **flags, size_t flag_count);
	Recipe &libraries(const char **libraries, size_t library_count);
	Recipe &files(Ingredients &files);
	std::string &get_name();

	Command get_command();
};

class Chef
{
  private:
	size_t m_Defaut_recipe;
	std::vector<Recipe> m_Recipes;

  public:
	Chef &default_recipe(const Recipe &recipe);
	Chef &learn_recipe(const Recipe &recipe);
	void cook();
	void cook(const std::string &name);

	void operator+=(const Recipe &recipe);
};

#ifndef _BUILD
#define _BUILD
enum class Heat { O0, O1, O2, O3, Ofast, Os, Oz, Og };

inline void start_job(Command command)
{
	pid_t pid = fork();
	if (pid == 0) {
		execvp(command.command.c_str(), command.args.data());
		std::cerr << "execvp failed to start: " << strerror(errno) << std::endl;
		exit(errno);
	} else if (pid > 0) {
		int status;

		if (waitpid(pid, &status, 0) == -1) {
			std::cerr << "waitpid failed: " << strerror(errno) << std::endl;
			exit(errno);
		}
	} else {
		std::cerr << "Failed to fork: " << strerror(errno) << std::endl;
		exit(errno);
	}
}

inline void Ingredients::operator+=(const std::string &file) { (void)add_source(file); }

inline Ingredients &Ingredients::prefix(const std::string &prefix)
{
	m_Prefix = prefix;
	return *this;
}

inline Ingredients &Ingredients::glob_source(const std::string &glob)
{
	TODO("implement globbing");
	return *this;
}

inline Ingredients &Ingredients::add_source(const std::string &file)
{
	this->m_Files.push_back(file);
	return *this;
}

inline std::vector<std::string> Ingredients::get_files()
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

inline Recipe &Recipe::compiler(const std::string &compiler)
{
	m_Compiler = compiler;
	return *this;
}

inline Recipe &Recipe::optimization(const Heat &level)
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

inline Recipe &Recipe::optimization(const std::string &level)
{
	std::string _level;
	if (level.find("-") == 0)
		_level = level;
	else
		_level = "-" + level;

	m_Optimization_level = _level;
	return *this;
}

inline Recipe &Recipe::cflags(const char **flags, size_t flag_count)
{
	for (size_t i = 0; i < flag_count; ++i)
		m_Cflags.push_back(flags[i]);
	return *this;
}

inline Recipe &Recipe::ldflags(const char **flags, size_t flag_count)
{
	for (size_t i = 0; i < flag_count; ++i)
		m_Ldflags.push_back(flags[i]);
	return *this;
}

inline Recipe &Recipe::libraries(const char **libraries, size_t library_count)
{
	for (size_t i = 0; i < library_count; ++i)
		m_Libs.push_back(libraries[i]);
	return *this;
}

inline Recipe &Recipe::output(const std::string &name)
{
	m_Output = name;
	return *this;
}

inline Command Recipe::get_command()
{
	assert(m_Compiler != "" && "ERROR: compiler is REQUIRED to COMPILE...\n");
	assert(m_Files.has_value() && "ERROR: you need to provide files to compile");

	Command command;
	command.command = m_Compiler;

	command.arg_strings.push_back(m_Compiler);

	for (auto &cflag : m_Cflags)
		command.arg_strings.push_back(cflag.c_str());

	if (m_Optimization_level != "")
		command.arg_strings.push_back(m_Optimization_level.c_str());

	for (auto &library : m_Libs)
		command.arg_strings.push_back(("-l" + library).c_str());

	if (m_Output != "") {
		command.arg_strings.push_back(("-o"));
		command.arg_strings.push_back(m_Output.c_str());
	}

	for (auto &file : m_Files->get_files())
		command.arg_strings.push_back(file.c_str());

	for (auto &ldflag : m_Ldflags)
		command.arg_strings.push_back(ldflag.c_str());

	for (auto &arg : command.arg_strings)
		command.args.push_back(arg.data());

	command.args.push_back(nullptr);

	return command;
}

inline Recipe &Recipe::files(Ingredients &files)
{
	m_Files = files;
	return *this;
}

inline std::string &Recipe::get_name() { return m_Name; }

inline Chef &Chef::default_recipe(const Recipe &recipe)
{
	m_Defaut_recipe = m_Recipes.size();
	m_Recipes.push_back(recipe);
	return *this;
}

inline Chef &Chef::learn_recipe(const Recipe &recipe)
{
	m_Recipes.push_back(recipe);
	return *this;
}

inline void Chef::operator+=(const Recipe &recipe)
{
	if (m_Recipes.size() == 0)
		default_recipe(recipe);
	else
		learn_recipe(recipe);
}

inline void Chef::cook()
{
	start_job(m_Recipes[m_Defaut_recipe].get_command());
}

inline void Chef::cook(const std::string &recipe_name)
{
	if (recipe_name == "")
		cook();

	for (auto recipe : m_Recipes)
		if (recipe.get_name() == recipe_name) {
			start_job(recipe.get_command());
			break;
		}
}

#endif // _BUILD
} // namespace Kitchen
