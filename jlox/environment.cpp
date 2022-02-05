#include "environment.hpp"

#include <lak/utility.hpp>

lox::environment *find_ancestor(lox::environment *env, size_t distance)
{
	while (env && distance-- > 0) env = env->enclosing.get();
	return env;
}

const lox::object &lox::environment::emplace(lak::u8string_view k,
                                             lox::object v)
{
	return values.insert_or_assign(k.to_string(), lak::move(v)).first->second;
}

const lox::object &lox::environment::emplace(const lox::token &k,
                                             lox::object v)
{
	return emplace(k.lexeme, lak::move(v));
}

const lox::object *lox::environment::find(lak::u8string_view k)
{
	if (auto it = values.find(k); it != values.end())
		return &it->second;
	else if (enclosing)
		return enclosing->find(k);
	else
		return nullptr;
}

const lox::object *lox::environment::find(lak::u8string_view k,
                                          size_t distance)
{
	lox::environment *env = find_ancestor(this, distance);
	return env ? env->find(k) : nullptr;
}

const lox::object *lox::environment::find(const lox::token &k)
{
	return find(k.lexeme);
}

const lox::object *lox::environment::find(const lox::token &k, size_t distance)
{
	return find(k.lexeme, distance);
}

const lox::object *lox::environment::replace(const lox::token &k,
                                             lox::object v)
{
	if (auto it = values.find(k.lexeme); it != values.end())
	{
		it->second = lak::move(v);
		return &it->second;
	}
	else if (enclosing)
		return enclosing->replace(k, lak::move(v));
	else
		return nullptr;
}

const lox::object *lox::environment::replace(const lox::token &k,
                                             lox::object v,
                                             size_t distance)
{
	lox::environment *env = find_ancestor(this, distance);
	return env ? env->replace(k, lak::move(v)) : nullptr;
}

lox::environment_ptr lox::environment::make(lox::environment_ptr enclosing)
{
	return lox::environment_ptr::make(lox::environment{.enclosing = enclosing});
}
