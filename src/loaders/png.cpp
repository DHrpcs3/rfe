#include <rfe/loaders/png.h>
#include <png.h>
#include <fstream>

namespace rfe
{
	namespace loaders
	{
		namespace png
		{
			std::shared_ptr<graphics::image> load(std::istream& stream)
			{
				png_byte header[8];
				stream.read((char*)header, sizeof(header));

				if (png_sig_cmp(header, 0, 8))
				{
					//TODO
					throw;
				}

				std::unique_ptr<png_struct, void(*)(png_structp)> png_ptr(png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr), [](png_structp png_ptr)
				{
					png_destroy_read_struct(&png_ptr, nullptr, nullptr);
				});


				//TODO: safe cleanup
				png_infop info_ptr = png_create_info_struct(png_ptr.get());

				if (setjmp(png_jmpbuf(png_ptr.get())))
				{
					throw;
				}

				png_set_read_fn(png_ptr.get(), &static_cast<std::istream&>(stream), [](png_structp png_ptr, png_bytep data, png_size_t size)
				{
					std::istream &stream = *((std::istream*)png_get_io_ptr(png_ptr));
					stream.read((char*)data, size);
				});

				png_set_sig_bytes(png_ptr.get(), 8);
				png_read_info(png_ptr.get(), info_ptr);

				png_uint_32 width, height;
				int bit_depth, color_type;

				png_get_IHDR(png_ptr.get(), info_ptr, &width, &height, &bit_depth, &color_type, nullptr, nullptr, nullptr);

				png_read_update_info(png_ptr.get(), info_ptr);

				if (setjmp(png_jmpbuf(png_ptr.get())))
				{
					throw;
				}

				png_size_t pitch = png_get_rowbytes(png_ptr.get(), info_ptr);

				std::unique_ptr<char[]> pixels = std::make_unique<char[]>(pitch * height);

				std::unique_ptr<png_bytep[]> rows = std::make_unique<png_bytep[]>(height);

				for (png_uint_32 i = 0; i < height; i++)
				{
					rows[height - 1 - i] = (png_byte*)pixels.get() + i * pitch;
				}

				png_read_image(png_ptr.get(), rows.get());

				graphics::pixels_type type;

				switch (color_type)
				{
				case PNG_COLOR_TYPE_RGB: type = graphics::pixels_type::rgb8; break;
				case PNG_COLOR_TYPE_RGB_ALPHA: type = graphics::pixels_type::rgba8; break;
				default:
					//todo
					throw;
				}

				png_destroy_info_struct(png_ptr.get(), &info_ptr);

				return std::make_shared<graphics::image>(std::move(pixels), size2i{ (int)width, (int)height }, type);
			}

			std::shared_ptr<graphics::image> load(const std::string &path)
			{
				std::ifstream stream(path, std::ios::binary);
				return load(stream);
			}
		}
	}
}
