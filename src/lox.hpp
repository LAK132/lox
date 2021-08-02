#ifndef LOX_LOX_HPP
#define LOX_LOX_HPP

#include <filesystem>
#include <string>
#include <system_error>
#include <variant>

namespace lox
{
	std::variant<std::u8string, std::error_code> read_file(
	  const std::filesystem::path &file_path);

	int usage();
}

#endif
