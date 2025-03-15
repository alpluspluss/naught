/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#pragma once

#include <naught/forge/buffer.hpp>

namespace nght::frg
{
	class UniformBuf : public Buffer
	{
	public:
		UniformBuf(Context& ctx, VkDeviceSize size);

		template<typename T>
		void update(const T& data)
		{
			void* mapped = map();
			memcpy(mapped, &data, sizeof(T));
			unmap();
		}
	};
}