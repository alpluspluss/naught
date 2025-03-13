#pragma once

namespace necs
{
	struct Column
	{
		void *data = nullptr;
		size_t size = 0;
		size_t capacity = 0;

		Column();

		~Column();

		Column(const Column &other);

		Column(Column &&other) noexcept;

		Column &operator=(const Column &other);

		Column &operator=(Column &&other) noexcept;

		void resize(size_t nsz);

		void clear();

		[[nodiscard]] void *get(size_t row) const;
	};
}
