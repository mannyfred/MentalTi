#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "Global.hpp"

namespace Utils {

	bool ParseUserKeywords(const std::string& input);
	bool ParseUserInput(const int argc, char** argv);
	bool CreateOpenOutputFile(const std::string& filename);
	void PrintHelp();
	bool EnablePPL();
	bool CtrlHandler(DWORD fdwCtrlType);
}

#endif // !UTILS_HPP
