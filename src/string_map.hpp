#ifndef LOX_STRING_MAP_HPP
#define LOX_STRING_MAP_HPP

#include <lak/functional.hpp>
#include <lak/result.hpp>
#include <lak/string.hpp>
#include <lak/string_view.hpp>
#include <lak/tuple.hpp>

#include <functional>
#include <string_view>
#include <unordered_map>

namespace lox
{
	template<typename CHAR>
	struct string_hash
	{
		using hash_type      = std::hash<std::basic_string_view<CHAR>>;
		using is_transparent = void;

		size_t operator()(const CHAR *str) const { return hash_type{}(str); }
		size_t operator()(lak::string_view<CHAR> str) const
		{
			return hash_type{}(std::basic_string_view<CHAR>(str.data(), str.size()));
		}
		size_t operator()(const lak::string<CHAR> &str) const
		{
			return hash_type{}(std::basic_string_view<CHAR>(str.data(), str.size()));
		}
	};

	template<typename CHAR, typename VALUE>
	using string_map = std::unordered_map<lak::string<CHAR>,
	                                      VALUE,
	                                      lox::string_hash<CHAR>,
	                                      lak::equal_to<>>;

	template<typename CHAR, typename VALUE, typename KEY>
	lak::result<std::pair<const lak::string<CHAR>, VALUE> &> find(
	  lox::string_map<CHAR, VALUE> &map, KEY &&key)
	{
		if (auto it{map.find(lak::forward<KEY>(key))}; it != map.end())
			return lak::ok_t{*it};
		else
			return lak::err_t{};
	}

	template<typename CHAR, typename VALUE, typename KEY>
	lak::result<const std::pair<const lak::string<CHAR>, VALUE> &> find(
	  const lox::string_map<CHAR, VALUE> &map, KEY &&key)
	{
		if (auto it{map.find(lak::forward<KEY>(key))}; it != map.end())
			return lak::ok_t{*it};
		else
			return lak::err_t{};
	}
}

#endif
