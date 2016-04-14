#pragma once
#include <vector>
#include <string>
#include <initializer_list>

namespace rfe
{
	inline namespace core
	{
		namespace fmt
		{
			std::vector<std::string> split(const std::string& source, std::initializer_list<const std::string> separators = { " ", "\t", "\n" }, bool is_skip_empty = true);
			std::vector<std::string> split(const std::string& source, const std::string& separator, bool is_skip_empty = true);

			template<typename T>
			std::string merge(const T& source, const std::string& separator = " ")
			{
				if (source.empty())
				{
					return{};
				}

				std::string result;

				auto end = source.end();
				--end;

				for (auto it = source.begin(); it != end; ++it)
				{
					result += *it + separator;
				}

				return result + source.back();
			}

			template<typename T>
			std::string merge(std::initializer_list<const T> sources, const std::string& separator = " ")
			{
				if (sources.empty())
				{
					return{};
				}

				std::string result = fmt::merge(sources.front(), separator);

				for (auto it = sources.begin() + 1; it != sources.end(); ++it)
				{
					result += separator + fmt::merge(*it, separator);
				}

				return result;
			}

			template<typename Iterator>
			auto join(Iterator begin, Iterator end)
			{
				typename std::remove_reference<decltype(*begin)>::type result;

				for (auto it = begin; it != end; ++it)
				{
					result += *it;
				}

				return result;
			}

			template<typename T, template<class> class Container>
			T join(const Container<T> &sources)
			{
				T result;

				for (auto &&source : sources)
				{
					result += source;
				}

				return result;
			}

			bool match(const std::string &src, const std::string& mask, std::vector<std::string> *masked_words = nullptr);

			template<typename... T>
			std::string format(const std::string& fmt, T... args)
			{
				std::vector<char> result(256);
				while (true)
				{
					int len = snprintf(result.data(), result.size(), fmt.c_str(), args...);

					if (len < 0 || (std::size_t)len >= result.size())
					{
						result.resize(size_t(result.size() * 1.5));
					}
					else
					{
						break;
					}
				}

				return result.data();
			}

			template<typename... T>
			std::wstring format(const std::wstring& fmt, T... args)
			{
				std::vector<wchar_t> result(256);
				while (true)
				{
					int len = swprintf(result.data(), result.size(), fmt.c_str(), args...);

					if (len < 0 || len >= result.size())
					{
						result.resize(size_t(result.size() * 1.5));
					}
					else
						break;
				}

				return result.data();
			}
		}
	}
}
