#pragma once

namespace core {
	template <typename T>
	class ColorRGBA {
	public:
		T red;
		T green;
		T blue;
		T alpha;

		ColorRGBA(T red = 0, T green = 0, T blue = 0, T alpha = 0) :
			red(red),
			green(green),
			blue(blue),
			alpha(alpha)
		{
		}
	};
}