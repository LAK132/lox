#ifndef LOX_STRING_MAP_HPP
#define LOX_STRING_MAP_HPP

#include <string>
#include <unordered_map>

namespace lox
{
	template<typename CHAR>
	struct string_hash
	{
		using hash_type      = std::hash<std::basic_string_view<CHAR>>;
		using is_transparent = void;

		size_t operator()(const CHAR *str) const { return hash_type{}(str); }
		size_t operator()(std::basic_string_view<CHAR> str) const
		{
			return hash_type{}(str);
		}
		size_t operator()(const std::basic_string<CHAR> &str) const
		{
			return hash_type{}(str);
		}
	};

	template<typename CHAR, typename VALUE>
	using string_map = std::unordered_map<std::basic_string<CHAR>,
	                                      VALUE,
	                                      lox::string_hash<CHAR>,
	                                      std::equal_to<>>;
}

#endif
