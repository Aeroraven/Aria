#pragma once
#include <cmath>
#include <vector>
#include <ranges>
namespace Anthem::Components::Math {

	class AnthemHaltonSequence {
	private:
		std::vector<float> seq;
		uint32_t size;
		uint32_t base;
	protected:
		void generate();
	public:
		class iterator{
		public:
			using iterator_category = std::forward_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = float;
			using reference = const float&;
			using pointer = float*;
		private:
			std::vector<float>::iterator baseIter;
		public:
			explicit iterator(std::vector<float>::iterator baseIter);
			iterator(const iterator& p);
			iterator& operator++();
			iterator operator++(int);
			bool operator==(iterator p) const;
			bool operator!=(iterator p) const;
			float& operator*() const;

		};
		AnthemHaltonSequence(uint32_t base, uint32_t size);
		float operator[](const uint32_t index) const;
		iterator begin();
		iterator end();
	};
}