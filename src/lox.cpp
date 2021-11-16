#include "lox.hpp"

#include "string_hacks.hpp"

#include <fstream>
#include <iostream>

std::variant<std::u8string, std::error_code> lox::read_file(
  const std::filesystem::path &file_path)
{
	try
	{
		std::ifstream file;
		file.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);
		file.open(file_path, std::ios::binary);
		std::ostringstream contents;
		contents << file.rdbuf();
		file.close();
		return std::u8string(
		  lox::as_u8string_view(std::string_view(contents.str())));
	}
	catch (const std::ios::failure &ex)
	{
		return ex.code();
	}
}

int lox::usage()
{
	std::cout << "Usage: lox [script]\n";
	return EXIT_FAILURE;
}
