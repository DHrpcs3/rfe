#include <rfe/core/fmt.h>
#include <cstring>

namespace rfe
{
	inline namespace core
	{
		namespace fmt
		{
			std::vector<std::string> split(const std::string& source, std::initializer_list<const std::string> separators, bool is_skip_empty)
			{
				std::vector<std::string> result;

				size_t cursor_begin = 0;

				for (size_t cursor_end = 0; cursor_end < source.length(); ++cursor_end)
				{
					for (auto &separator : separators)
					{
						if (std::strncmp(source.c_str() + cursor_end, separator.c_str(), separator.length()) == 0)
						{
							std::string candidate = source.substr(cursor_begin, cursor_end - cursor_begin);
							if (!is_skip_empty || !candidate.empty())
								result.push_back(candidate);

							cursor_begin = cursor_end + separator.length();
							cursor_end = cursor_begin - 1;
							break;
						}
					}
				}

				if (cursor_begin != source.length())
				{
					result.push_back(source.substr(cursor_begin));
				}

				return result;
			}

			std::vector<std::string> split(const std::string& source, const std::string& separator, bool is_skip_empty)
			{
				return split(source, { separator }, is_skip_empty);
			}

			bool match(const std::string &src, const std::string& mask, std::vector<std::string> *masked_words)
			{
				if (mask.empty())
					return true;

				size_t src_index = 0;

				for (size_t i = 0; i < mask.size(); ++i)
				{
					if (src_index >= src.size())
						return false;

					switch (mask[i])
					{
					case '\\':
						if (mask[i + 1] == '\\')
						{
							continue;
						}

						++i;
						break;

					case '?':
						++src_index;
						continue;

					case '*':
					{
						std::string new_mask = mask.substr(i + 1);

						if (new_mask.empty())
						{
							if (masked_words)
								masked_words->push_back(src.substr(src_index));

							return true;
						}

						for (size_t j = src_index; j < src.size(); ++j)
						{
							std::vector<std::string> tmp_masked;
							if (match(src.c_str() + j, new_mask, &tmp_masked))
							{
								if (masked_words)
								{
									if (j != src_index)
										masked_words->push_back(src.substr(src_index, j - src_index));

									for (auto &masked : tmp_masked)
									{
										masked_words->push_back(masked);
									}
								}

								return true;
							}
						}

						return false;
					}
					}

					if (mask[i] != src[src_index++])
					{
						return false;
					}
				}

				return src_index >= src.size();
			}
		}
	}
}
