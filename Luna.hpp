#pragma once
#include <tuple>
#include <format>
#include <string>
#include <cstdint>
#include <exception>
#include <functional>
#include <type_traits>

#include <lua.hpp>

enum LUNA_LIBRARY
{
	LUNA_LIBRARY_IO        = LUA_IOLIBK,
	LUNA_LIBRARY_OS        = LUA_OSLIBK,
	LUNA_LIBRARY_BASE      = LUA_GLIBK,
	LUNA_LIBRARY_MATH      = LUA_MATHLIBK,
	LUNA_LIBRARY_UTF8      = LUA_UTF8LIBK,
	LUNA_LIBRARY_DEBUG     = LUA_DBLIBK,
	LUNA_LIBRARY_TABLE     = LUA_TABLIBK,
	LUNA_LIBRARY_STRING    = LUA_STRLIBK,
	LUNA_LIBRARY_PACKAGE   = LUA_LOADLIBK,
	LUNA_LIBRARY_COROUTINE = LUA_COLIBK
};

class Luna;

template<typename F>
class LunaFunction;

class LunaTable
{
	friend Luna;

	struct Context
	{
		size_t     RefCount;

		lua_State* Lua_State;
		bool       Lua_Ownership;
		int        Lua_Reference;

		~Context()
		{
			if (Lua_Ownership)
				luaL_unref(Lua_State, LUA_REGISTRYINDEX, Lua_Reference);
		}
	};

	Context* context;

public:
	LunaTable()
		: context(nullptr)
	{
	}
	LunaTable(LunaTable&& table)
		: context(table.context)
	{
		table.context = nullptr;
	}
	LunaTable(const LunaTable& table)
		: context(table.context)
	{
		if (context)
			++context->RefCount;
	}
	LunaTable(lua_State* lua, int reference, bool take_ownership)
		: context(new Context{
			.RefCount      = 1,
			.Lua_State     = lua,
			.Lua_Ownership = take_ownership,
			.Lua_Reference = reference
		})
	{
	}

	virtual ~LunaTable()
	{
		Release();
	}

	// @throw LunaException
	template<typename T>
	bool GetField(const char* name, T& value) const;

	template<typename T>
	bool SetField(const char* name, const T& value);

	void Release()
	{
		if (context)
		{
			if (!--context->RefCount)
				delete context;

			context = nullptr;
		}
	}

	operator bool        () const
	{
		return context != nullptr;
	}
	operator int         () const
	{
		return context ? context->Lua_Reference : LUA_NOREF;
	}
	operator lua_Integer () const
	{
		return context ? context->Lua_Reference : LUA_NOREF;
	}

	auto& operator = (LunaTable&& table)
	{
		Release();

		context = table.context;
		table.context = nullptr;

		return *this;
	}
	auto& operator = (const LunaTable& table)
	{
		Release();

		if (table.context)
		{
			context = table.context;

			++context->RefCount;
		}

		return *this;
	}

	bool operator == (const LunaTable& table) const
	{
		if (context == table.context)
			return true;

		if (context && table.context)
			return context->Lua_Reference == table.context->Lua_Reference;

		return false;
	}
	bool operator != (const LunaTable& table) const
	{
		return context != table.context;
	}
};

class LunaLibrary
{
	friend Luna;

	struct Context
	{
		bool       IsLoaded;

		size_t     RefCount;

		lua_State* Lua_State;
		bool       Lua_Ownership;
		int        Lua_Reference;

		~Context()
		{
			if (Lua_Ownership)
				luaL_unref(Lua_State, LUA_REGISTRYINDEX, Lua_Reference);
		}
	};

	Context* context;

public:
	LunaLibrary()
		: context(nullptr)
	{
	}
	LunaLibrary(LunaLibrary&& library)
		: context(library.context)
	{
		library.context = nullptr;
	}
	LunaLibrary(const LunaLibrary& library)
		: context(library.context)
	{
		if (context)
			++context->RefCount;
	}
	LunaLibrary(lua_State* lua, int reference, bool take_ownership)
		: context(new Context{
			.RefCount      = 1,
			.Lua_State     = lua,
			.Lua_Ownership = take_ownership,
			.Lua_Reference = reference
		})
	{
	}

	bool IsLoaded() const
	{
		return context && context->IsLoaded;
	}

	// @throw LunaException
	template<typename T>
	bool GetField(const char* name, T& value) const;

	template<typename T>
	bool SetField(const char* name, const T& value);

	void Release()
	{
		if (context)
		{
			if (!--context->RefCount)
				delete context;

			context = nullptr;
		}
	}

	operator bool        () const
	{
		return context != nullptr;
	}
	operator int         () const
	{
		return context ? context->Lua_Reference : LUA_NOREF;
	}
	operator lua_Integer () const
	{
		return context ? context->Lua_Reference : LUA_NOREF;
	}

	auto& operator = (LunaLibrary&& library)
	{
		Release();

		context = library.context;
		library.context = nullptr;

		return *this;
	}
	auto& operator = (const LunaLibrary& library)
	{
		Release();

		if (library.context)
		{
			context = library.context;

			++context->RefCount;
		}

		return *this;
	}

	bool operator == (const LunaLibrary& library) const
	{
		if (context == library.context)
			return true;

		if (context && library.context)
			return context->Lua_Reference == library.context->Lua_Reference;

		return false;
	}
	bool operator != (const LunaLibrary& library) const
	{
		return context != library.context;
	}
};

template<typename T, typename ... TArgs>
class LunaFunction<T(TArgs ...)>
{
	friend Luna;

	struct Context
	{
		int                         Type;
		size_t                      RefCount;

		T(*                         C_Function)(TArgs ...);

		std::function<T(TArgs ...)> CPP_Function;

		lua_State*                  Lua_State;
		bool                        Lua_Ownership;
		int                         Lua_Reference;

		~Context()
		{
			if (Lua_Ownership)
				luaL_unref(Lua_State, LUA_REGISTRYINDEX, Lua_Reference);
		}
	};

	Context* context;

public:
	LunaFunction()
		: context(nullptr)
	{
	}
	template<typename F>
	LunaFunction(F&& function);
	LunaFunction(T(*function)(TArgs ...));
	LunaFunction(LunaFunction&& function)
		: context(function.context)
	{
		function.context = nullptr;
	}
	LunaFunction(const LunaFunction& function)
		: context(function.context)
	{
		if (context)
			++context->RefCount;
	}
	LunaFunction(lua_State* lua, int reference, bool take_ownership);

	virtual ~LunaFunction()
	{
		Release();
	}

	void Release()
	{
		if (context)
		{
			if (!--context->RefCount)
				delete context;

			context = nullptr;
		}
	}

	operator bool        () const
	{
		return context != nullptr;
	}
	operator int         () const;
	operator lua_Integer () const;

	// @throw LunaException
	T     operator () (TArgs ... args) const;

	auto& operator = (LunaFunction&& function)
	{
		Release();

		context = function.context;
		function.context = nullptr;

		return *this;
	}
	auto& operator = (const LunaFunction& function)
	{
		Release();

		if (function.context)
		{
			context = function.context;

			++context->RefCount;
		}

		return *this;
	}

	bool operator == (const LunaFunction& function) const
	{
		if (context == function.context)
			return true;

		if (context && function.context)
			return context->Lua_Reference == function.context->Lua_Reference;

		return false;
	}
	bool operator != (const LunaFunction& function) const
	{
		return context != function.context;
	}

private:
	// @throw LunaException
	static int C(lua_State* lua)
	{
		return C(lua, std::make_index_sequence<sizeof...(TArgs)> {});
	}
	// @throw LunaException
	template<size_t ... I>
	static int C(lua_State* lua, std::index_sequence<I ...>);

	// @throw LunaException
	static int CPP(lua_State* lua)
	{
		return CPP(lua, std::make_index_sequence<sizeof...(TArgs)> {});
	}
	// @throw LunaException
	template<size_t ... I>
	static int CPP(lua_State* lua, std::index_sequence<I ...>);
};

class LunaException
	: public std::exception
{
	std::string message;

public:
	LunaException()
	{
	}
	LunaException(std::string&& message)
		: message(std::move(message))
	{
	}
	LunaException(const char* function, int result)
		: message(std::format("Error calling '{}' -> {}", function, result))
	{
	}
	LunaException(const char* function, lua_State* lua)
		: LunaException(function, lua_tostring(lua, -1))
	{
		lua_pop(lua, 1);
	}
	LunaException(const char* function, const char* message)
		: message(std::format("Error calling '{}': {}", function, message))
	{
	}

	virtual const char* what() const noexcept
	{
		return message.c_str();
	}
};

class Luna
{
	friend class LunaTable;
	friend class LunaLibrary;
	template<typename F>
	friend class LunaFunction;

	enum FUNCTION_TYPE
	{
		FUNCTION_TYPE_C,
		FUNCTION_TYPE_CPP,
		FUNCTION_TYPE_LUA
	};

	lua_State* lua;
	bool       lua_is_owned;

	Luna(Luna&&) = delete;
	Luna(const Luna&) = delete;

public:
	Luna()
		: lua(luaL_newstate()),
		lua_is_owned(true)
	{
	}
	Luna(lua_State* lua)
		: lua(lua),
		lua_is_owned(false)
	{
	}
	Luna(lua_Alloc alloc, void* ud)
		: lua(lua_newstate(alloc, ud, luaL_makeseed(NULL))),
		lua_is_owned(true)
	{
	}

	virtual ~Luna()
	{
		if (lua && lua_is_owned)
		{
			lua_close(lua);

			lua = nullptr;
		}
	}

	// @throw LunaException
	bool Run(const char* lua)
	{
		if (!this->lua || !lua)
			return false;

		if (luaL_dostring(this->lua, lua))
			throw LunaException("luaL_dostring", this->lua);

		return true;
	}
	// @throw LunaException
	bool RunFile(const char* path)
	{
		if (!lua || !path)
			return false;

		if (luaL_dofile(lua, path))
			throw LunaException("luaL_dofile", lua);

		return true;
	}

	// @throw LunaException
	template<typename T>
	bool GetGlobal(const char* name, T& value) const
	{
		if (!lua || !name)
			return false;

		if (lua_getglobal(lua, name) == LUA_TNONE)
			return false;

		Stack_Pop(lua, value, false);

		return true;
	}

	template<typename T>
	bool SetGlobal(const char* name, const T& value)
	{
		if (!lua || !name)
			return false;

		Stack_Push(lua, value, false);
		lua_setglobal(lua, name);

		return true;
	}

	bool LoadLibrary(int mask)
	{
		if (!lua)
			return false;

		luaL_openselectedlibs(lua, mask, 0);

		return true;
	}
	bool LoadLibrary(const LunaLibrary& value, const char* name, bool global)
	{
		if (!lua || !value || value.IsLoaded())
			return false;

		luaL_getsubtable(lua, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
		lua_rawgeti(lua, LUA_REGISTRYINDEX, value);
		lua_setfield(lua, -2, name);
		lua_pop(lua, 1);

		if (global)
		{
			lua_rawgeti(lua, LUA_REGISTRYINDEX, value);
			lua_setglobal(lua, name);
		}

		value.context->IsLoaded = true;

		return true;
	}

	auto CreateTable()
	{
		LunaTable value;

		if (lua)
		{
			lua_newtable(lua);

			int reference;

			if ((reference = luaL_ref(lua, LUA_REGISTRYINDEX)) == LUA_NOREF)
				lua_pop(lua, 1);
			else
				value = LunaTable(lua, reference, true);
		}

		return value;
	}

	auto CreateLibrary()
	{
		LunaLibrary value;

		if (lua)
		{
			lua_newtable(lua);

			int reference;

			if ((reference = luaL_ref(lua, LUA_REGISTRYINDEX)) == LUA_NOREF)
				lua_pop(lua, 1);
			else
				value = LunaLibrary(lua, reference, true);
		}

		return value;
	}

	bool RemoveGlobal(const char* name)
	{
		if (!lua || !name)
			return false;

		lua_pushnil(lua);
		lua_setglobal(lua, name);

		return true;
	}

	constexpr operator bool () const
	{
		return lua != nullptr;
	}

	constexpr operator lua_State* () const
	{
		return lua;
	}

private:
	// @throw LunaException
	template<typename T>
	static T    Stack_Pop(lua_State* lua, bool p)
	{
		T value;

		Stack_Pop(lua, value, p);

		return value;
	}
	// @throw LunaException
	template<typename T>
	static void Stack_Pop(lua_State* lua, T& value, bool p)
	{
		value = Stack_Peek<T>(lua, -1, p);

		lua_pop(lua, 1);
	}
	static void Stack_Pop(lua_State* lua, int count)
	{
		lua_pop(lua, count);
	}

	// @throw LunaException
	template<typename T>
	static T    Stack_Peek(lua_State* lua, int index, bool p)
	{
		T value;

		Stack_Peek(lua, index, value, p);

		return value;
	}
	// @throw LunaException
	template<typename T>
	static void Stack_Peek(lua_State* lua, int index, T& value, bool p)
	{
		if (!lua_isnumber(lua, index))
		{
			auto type = lua_type(lua, index);

			if (type == LUA_TNIL)
				value = (T)0;
			else if ((type == LUA_TSTRING) && std::is_integral<T>::value)
			{
				const char* string;
				size_t      string_length;

				if ((string = lua_tolstring(lua, index, &string_length)) == nullptr)
					value = (T)0;
				else if (string_length == 1)
					value = (T)*string;
				else
				{
					if (p)
						luaL_error(lua, "String length must be 1 at index %d", index);

					throw LunaException(std::format("String length must be 1 at index {}", index));
				}
			}
			else
			{
				auto type_name = lua_typename(lua, type);

				if (p)
					luaL_error(lua, "Invalid type at index %d. Found %s, expected %s", index, type_name, lua_typename(lua, LUA_TNUMBER));

				throw LunaException(std::format("Invalid type at index {}. Found {}, expected {}", index, type_name, lua_typename(lua, LUA_TNUMBER)));
			}
		}
		else if constexpr (std::is_floating_point<T>::value)
			value = (T)lua_tonumber(lua, index);
		else if constexpr (std::is_enum<T>::value || std::is_integral<T>::value)
			value = (T)lua_tointeger(lua, index);
		else
			static_assert(false, "Type not supported");
	}
	// @throw LunaException
	template<typename T>
	static void Stack_Peek(lua_State* lua, int index, T*& value, bool p)
	{
		if (!lua_islightuserdata(lua, index))
		{
			auto type      = lua_type(lua, index);
			auto type_name = lua_typename(lua, type);

			if (p)
				luaL_error(lua, "Invalid type at index %d. Found %s, expected %s", index, type_name, lua_typename(lua, LUA_TLIGHTUSERDATA));

			throw LunaException(std::format("Invalid type at index {}. Found {}, expected {}", index, type_name, lua_typename(lua, LUA_TLIGHTUSERDATA)));
		}

		value = (T*)lua_touserdata(lua, index);
	}
	// @throw LunaException
	static void Stack_Peek(lua_State* lua, int index, bool& value, bool p)
	{
		if (!lua_isboolean(lua, index))
		{
			auto type      = lua_type(lua, index);
			auto type_name = lua_typename(lua, type);

			if (p)
				luaL_error(lua, "Invalid type at index %d. Found %s, expected %s", index, type_name, lua_typename(lua, LUA_TBOOLEAN));

			throw LunaException(std::format("Invalid type at index {}. Found {}, expected {}", index, type_name, lua_typename(lua, LUA_TBOOLEAN)));
		}

		value = lua_toboolean(lua, index) != 0;
	}
	// @throw LunaException
	static void Stack_Peek(lua_State* lua, int index, char& value, bool p)
	{
		if (!lua_isstring(lua, index))
		{
			auto type      = lua_type(lua, index);
			auto type_name = lua_typename(lua, type);

			if (p)
				luaL_error(lua, "Invalid type at index %d. Found %s, expected %s", index, type_name, lua_typename(lua, LUA_TSTRING));

			throw LunaException(std::format("Invalid type at index {}. Found {}, expected {}", index, type_name, lua_typename(lua, LUA_TSTRING)));
		}

		const char* string;
		size_t      string_length;

		if ((string = lua_tolstring(lua, index, &string_length)) == nullptr)
			string_length = 0;

		if (string_length != 1)
		{
			if (p)
				luaL_error(lua, "String length must be 1 at index %d", index);

			throw LunaException("String length must be 1 at index {}", index);
		}

		value = string_length ? *string : 0;
	}
	// @throw LunaException
	static void Stack_Peek(lua_State* lua, int index, char*& value, bool p)
	{
		if (lua_isnil(lua, index))
			value = nullptr;
		else
		{
			if (!lua_isstring(lua, index))
			{
				auto type      = lua_type(lua, index);
				auto type_name = lua_typename(lua, type);

				if (p)
					luaL_error(lua, "Invalid type at index %d. Found %s, expected %s", index, type_name, lua_typename(lua, LUA_TSTRING));

				throw LunaException(std::format("Invalid type at index {}. Found {}, expected {}", index, type_name, lua_typename(lua, LUA_TSTRING)));
			}

			value = (char*)lua_tostring(lua, index);
		}
	}
	// @throw LunaException
	static void Stack_Peek(lua_State* lua, int index, const char*& value, bool p)
	{
		if (lua_isnil(lua, index))
			value = nullptr;
		else
		{
			if (!lua_isstring(lua, index))
			{
				auto type      = lua_type(lua, index);
				auto type_name = lua_typename(lua, type);

				if (p)
					luaL_error(lua, "Invalid type at index %d. Found %s, expected %s", index, type_name, lua_typename(lua, LUA_TSTRING));

				throw LunaException(std::format("Invalid type at index {}. Found {}, expected {}", index, type_name, lua_typename(lua, LUA_TSTRING)));
			}

			value = lua_tostring(lua, index);
		}
	}
	// @throw LunaException
	static void Stack_Peek(lua_State* lua, int index, std::string& value, bool p)
	{
		if (lua_isnil(lua, index))
			value.clear();
		else
		{
			if (!lua_isstring(lua, index))
			{
				auto type      = lua_type(lua, index);
				auto type_name = lua_typename(lua, type);

				if (p)
					luaL_error(lua, "Invalid type at index %d. Found %s, expected %s", index, type_name, lua_typename(lua, LUA_TSTRING));

				throw LunaException(std::format("Invalid type at index {}. Found {}, expected {}", index, type_name, lua_typename(lua, LUA_TSTRING)));
			}

			const char* string;
			size_t      string_length;

			if ((string = lua_tolstring(lua, index, &string_length)) == nullptr)
				string_length = 0;

			value.assign(string, string_length);
		}
	}
	// @throw LunaException
	static void Stack_Peek(lua_State* lua, int index, std::string_view& value, bool p)
	{
		if (lua_isnil(lua, index))
			value = std::string_view();
		else
		{
			if (!lua_isstring(lua, index))
			{
				auto type      = lua_type(lua, index);
				auto type_name = lua_typename(lua, type);

				if (p)
					luaL_error(lua, "Invalid type at index %d. Found %s, expected %s", index, type_name, lua_typename(lua, LUA_TSTRING));

				throw LunaException(std::format("Invalid type at index {}. Found {}, expected {}", index, type_name, lua_typename(lua, LUA_TSTRING)));
			}

			const char* string;
			size_t      string_length;

			if ((string = lua_tolstring(lua, index, &string_length)) == nullptr)
				string_length = 0;

			value = std::string_view(string, string_length);
		}
	}
	// @throw LunaException
	static void Stack_Peek(lua_State* lua, int index, LunaTable& value, bool p)
	{
		if (lua_isnil(lua, index))
			value = LunaTable();
		else if (!lua_istable(lua, index))
		{
			auto type      = lua_type(lua, index);
			auto type_name = lua_typename(lua, type);

			if (p)
				luaL_error(lua, "Invalid type at index %d. Found %s, expected %s", index, type_name, lua_typename(lua, LUA_TTABLE));

			throw LunaException(std::format("Invalid type at index {}. Found {}, expected {}", index, type_name, lua_typename(lua, LUA_TTABLE)));
		}

		int reference;

		if ((reference = luaL_ref(lua, LUA_REGISTRYINDEX)) == LUA_NOREF)
		{
			lua_pop(lua, 1);

			if (p)
				luaL_error(lua, "luaL_ref returned %d", reference);

			throw LunaException("luaL_ref", reference);
		}

		value = LunaTable(lua, reference, true);
	}
	// @throw LunaException
	template<typename F>
	static void Stack_Peek(lua_State* lua, int index, LunaFunction<F>& value, bool p)
	{
		if (!lua_isfunction(lua, index))
		{
			auto type      = lua_type(lua, index);
			auto type_name = lua_typename(lua, type);

			if (p)
				luaL_error(lua, "Invalid type at index %d. Found %s, expected %s", index, type_name, lua_typename(lua, LUA_TFUNCTION));

			throw LunaException(std::format("Invalid type at index {}. Found {}, expected {}", index, type_name, lua_typename(lua, LUA_TFUNCTION)));
		}

		lua_pushvalue(lua, index);

		int reference;

		if ((reference = luaL_ref(lua, LUA_REGISTRYINDEX)) == LUA_NOREF)
		{
			lua_pop(lua, 1);

			if (p)
				luaL_error(lua, "luaL_ref returned %d", reference);

			throw LunaException("luaL_ref", reference);
		}

		value = LunaFunction<F>(lua, reference, true);
	}

	template<typename T>
	static int  Stack_Push(lua_State* lua, T value, bool p)
	{
		if constexpr (std::is_floating_point<T>::value)
		{
			lua_pushnumber(lua, (lua_Number)value);

			return 1;
		}
		else if constexpr (std::is_enum<T>::value || std::is_integral<T>::value)
		{
			lua_pushinteger(lua, (lua_Integer)value);

			return 1;
		}
		else
			static_assert(false, "Type not supported");
	}
	template<typename T>
	static int  Stack_Push(lua_State* lua, T* value, bool p)
	{
		if (value == nullptr)
			lua_pushnil(lua);
		else
			lua_pushlightuserdata(lua, (void*)value);

		return 1;
	}
	static int  Stack_Push(lua_State* lua, bool value, bool p)
	{
		lua_pushboolean(lua, value ? 1 : 0);

		return 1;
	}
	static int  Stack_Push(lua_State* lua, char value, bool p)
	{
		lua_pushlstring(lua, &value, 1);

		return 1;
	}
	static int  Stack_Push(lua_State* lua, char* value, bool p)
	{
		if (value == nullptr)
			lua_pushnil(lua);
		else
			lua_pushstring(lua, value);

		return 1;
	}
	static int  Stack_Push(lua_State* lua, const char* value, bool p)
	{
		if (value == nullptr)
			lua_pushnil(lua);
		else
			lua_pushstring(lua, value);

		return 1;
	}
	static int  Stack_Push(lua_State* lua, std::string_view value, bool p)
	{
		lua_pushexternalstring(lua, value.data(), value.length(), nullptr, nullptr);

		return 1;
	}
	static int  Stack_Push(lua_State* lua, const std::string& value, bool p)
	{
		lua_pushexternalstring(lua, value.c_str(), value.length(), nullptr, nullptr);

		return 1;
	}
	static int  Stack_Push(lua_State* lua, const LunaTable& value, bool p)
	{
		if (!value)
		{
			lua_pushnil(lua);

			return 1;
		}

		lua_rawgeti(lua, LUA_REGISTRYINDEX, value.context->Lua_Reference);

		return 1;
	}
	template<typename T, typename ... TArgs>
	static int  Stack_Push(lua_State* lua, T(*value)(TArgs ...), bool p)
	{
		if (!value)
		{
			lua_pushnil(lua);

			return 1;
		}

		lua_pushlightuserdata(lua, (void*)value);
		lua_pushcclosure(lua, &LunaFunction<T(TArgs ...)>::C, 1);

		return 1;
	}
	template<typename T, typename ... TArgs>
	static int  Stack_Push(lua_State* lua, const LunaFunction<T(TArgs ...)>& value, bool p)
	{
		if (!value)
		{
			lua_pushnil(lua);

			return 1;
		}

		lua_pushlightuserdata(lua, (void*)&value);
		lua_pushcclosure(lua, &LunaFunction<T(TArgs ...)>::CPP, 1);

		return 1;
	}
	template<typename F>
	static int  Stack_Push(lua_State* lua, const LunaFunction<F>& value, bool p)
	{
		if (!value)
		{
			lua_pushnil(lua);

			return 1;
		}

		switch (value.context->Type)
		{
			case FUNCTION_TYPE_C:
				lua_pushlightuserdata(lua, (void*)value.context->C_Function);
				lua_pushcclosure(lua, &LunaFunction<F>::C, 1);
				break;

			case FUNCTION_TYPE_CPP:
				lua_pushlightuserdata(lua, (void*)&value.context->CPP_Function);
				lua_pushcclosure(lua, &LunaFunction<F>::CPP, 1);
				break;

			case FUNCTION_TYPE_LUA:
				lua_rawgeti(lua, LUA_REGISTRYINDEX, value.context->Lua_Reference);
				break;
		}

		return 1;
	}
	template<typename ... T>
	static int  Stack_Push(lua_State* lua, const std::tuple<T ...>& value, bool p)
	{
		return Stack_Push(lua, value, p, std::make_index_sequence<sizeof...(T)> {});
	}
	template<typename ... T, size_t ... I>
	static int  Stack_Push(lua_State* lua, const std::tuple<T ...>& value, bool p, std::index_sequence<I ...>)
	{
		return (Stack_Push(lua, std::get<I>(value), p) + ...);
	}
};

template<typename T>
inline bool LunaTable::GetField(const char* name, T& value) const
{
	if (!context || !name)
		return false;

	lua_rawgeti(context->Lua_State, LUA_REGISTRYINDEX, context->Lua_Reference);
	Luna::Stack_Pop(context->Lua_State, value, false);
	lua_pop(context->Lua_State, 1);

	return true;
}
template<typename T>
inline bool LunaTable::SetField(const char* name, const T& value)
{
	if (!context || !name)
		return false;

	lua_rawgeti(context->Lua_State, LUA_REGISTRYINDEX, context->Lua_Reference);
	Luna::Stack_Push(context->Lua_State, value, false);
	lua_setfield(context->Lua_State, -2, name);
	lua_pop(context->Lua_State, 1);

	return true;
}

template<typename T>
inline bool LunaLibrary::GetField(const char* name, T& value) const
{
	if (!context || !name)
		return false;

	lua_rawgeti(context->Lua_State, LUA_REGISTRYINDEX, context->Lua_Reference);
	Luna::Stack_Pop(context->Lua_State, value, false);
	lua_pop(context->Lua_State, 1);

	return true;
}
template<typename T>
inline bool LunaLibrary::SetField(const char* name, const T& value)
{
	if (!context || !name)
		return false;

	lua_rawgeti(context->Lua_State, LUA_REGISTRYINDEX, context->Lua_Reference);
	Luna::Stack_Push(context->Lua_State, value, false);
	lua_setfield(context->Lua_State, -2, name);
	lua_pop(context->Lua_State, 1);

	return true;
}

template<typename T, typename ... TArgs>
template<typename F>
inline LunaFunction<T(TArgs ...)>::LunaFunction(F&& function)
	: context(new Context{
		.Type         = Luna::FUNCTION_TYPE_CPP,
		.RefCount     = 1,
		.CPP_Function = std::move(function)
	})
{
}
template<typename T, typename ... TArgs>
inline LunaFunction<T(TArgs ...)>::LunaFunction(T(*function)(TArgs ...))
	: context(new Context{
		.Type       = Luna::FUNCTION_TYPE_C,
		.RefCount   = 1,
		.C_Function = function
	})
{
}
template<typename T, typename ... TArgs>
inline LunaFunction<T(TArgs ...)>::LunaFunction(lua_State* lua, int reference, bool take_ownership)
	: context(new Context{
		.Type          = Luna::FUNCTION_TYPE_LUA,
		.RefCount      = 1,
		.Lua_State     = lua,
		.Lua_Ownership = take_ownership,
		.Lua_Reference = reference
	})
{
}
template<typename T, typename ... TArgs>
inline LunaFunction<T(TArgs ...)>::operator int () const
{
	return (context && (context->Type == Luna::FUNCTION_TYPE_LUA)) ? context->Lua_Reference : LUA_NOREF;
}
template<typename T, typename ... TArgs>
inline LunaFunction<T(TArgs ...)>::operator lua_Integer () const
{
	return (context && (context->Type == Luna::FUNCTION_TYPE_LUA)) ? context->Lua_Reference : LUA_NOREF;
}
template<typename T, typename ... TArgs>
inline T LunaFunction<T(TArgs ...)>::operator () (TArgs ... args) const
{
	if (context)
	{
		switch (context->Type)
		{
			case Luna::FUNCTION_TYPE_C:
				return context->C_Function(std::forward<TArgs>(args) ...);

			case Luna::FUNCTION_TYPE_CPP:
				return context->CPP_Function(std::forward<TArgs>(args) ...);
		}

		if (lua_rawgeti(context->Lua_State, LUA_REGISTRYINDEX, context->Lua_Reference) != LUA_TFUNCTION)
			throw LunaException("lua_rawgeti", context->Lua_State);

		if constexpr (sizeof...(TArgs) == 0)
		{
			if constexpr (std::is_same<T, void>::value)
			{
				if (lua_pcall(context->Lua_State, 0, 0, 0) != LUA_OK)
					throw LunaException("lua_pcall", context->Lua_State);
			}
			else
			{
				if (lua_pcall(context->Lua_State, 0, LUA_MULTRET, 0) != LUA_OK)
					throw LunaException("lua_pcall", context->Lua_State);

				return Luna::Stack_Pop<T>(context->Lua_State, false);
			}
		}
		else if constexpr (std::is_same<T, void>::value)
		{
			if (lua_pcall(context->Lua_State, (Luna::Stack_Push(context->Lua_State, args, false) + ...), 0, 0) != LUA_OK)
				throw LunaException("lua_pcall", context->Lua_State);
		}
		else
		{
			if (lua_pcall(context->Lua_State, (Luna::Stack_Push(context->Lua_State, args, false) + ...), LUA_MULTRET, 0) != LUA_OK)
				throw LunaException("lua_pcall", context->Lua_State);

			return Luna::Stack_Pop<T>(context->Lua_State, false);
		}
	}

	return T();
}
template<typename T, typename ... TArgs>
template<size_t ... I>
inline int LunaFunction<T(TArgs ...)>::C(lua_State* lua, std::index_sequence<I ...>)
{
	auto function = (T(*)(TArgs ...))lua_touserdata(lua, lua_upvalueindex(1));

	if constexpr (std::is_same<T, void>::value)
		return function(Luna::Stack_Peek<typename std::tuple_element<I, std::tuple<TArgs ...>>::type>(lua, I + 1, true) ...), 0;
	else
		return Luna::Stack_Push(lua, function(Luna::Stack_Peek<typename std::tuple_element<I, std::tuple<TArgs ...>>::type>(lua, I + 1, true) ...), true);
}
template<typename T, typename ... TArgs>
template<size_t ... I>
inline int LunaFunction<T(TArgs ...)>::CPP(lua_State* lua, std::index_sequence<I ...>)
{
	auto function = (std::function<T(TArgs ...)>*)lua_touserdata(lua, lua_upvalueindex(1));

	if constexpr (std::is_same<T, void>::value)
		return (*function)(Luna::Stack_Peek<typename std::tuple_element<I, std::tuple<TArgs ...>>::type>(lua, I + 1, true) ...), 0;
	else
		return Luna::Stack_Push(lua, (*function)(Luna::Stack_Peek<typename std::tuple_element<I, std::tuple<TArgs ...>>::type>(lua, I + 1, true) ...), true);
}
