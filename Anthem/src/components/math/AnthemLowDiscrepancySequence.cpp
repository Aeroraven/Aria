#include "../../../include/components/math/AnthemLowDiscrepancySequence.h"

namespace Anthem::Components::Math {
	AnthemHaltonSequence::iterator::iterator(std::vector<float>::iterator baseIter) {
		this->baseIter = baseIter;
	}
	AnthemHaltonSequence::iterator::iterator(const iterator& p) {
		this->baseIter = p.baseIter;
	}
	AnthemHaltonSequence::iterator& AnthemHaltonSequence::iterator::operator++() {
		this->baseIter++;
		return *this;
	}
	AnthemHaltonSequence::iterator AnthemHaltonSequence::iterator::operator++(int) {
		auto retval = *this;
		this->baseIter++;
		return retval;
	}
	bool AnthemHaltonSequence::iterator::operator==(AnthemHaltonSequence::iterator p) const {
		return this->baseIter == p.baseIter;
	}
	bool AnthemHaltonSequence::iterator::operator!=(AnthemHaltonSequence::iterator p) const {
		return this->baseIter != p.baseIter;
	}
	float& AnthemHaltonSequence::iterator::operator*() const{
		return *baseIter;
	}
	void AnthemHaltonSequence::generate() {
		uint32_t n = 0, d = 1;
		for (uint32_t i = 0; i < size; i++) {
			auto x = d - n;
			if (x == 1) {
				n = 1;
				d *= base;
			}
			else {
				auto y = d / base;
				while (x <= y) {
					y /= base;
				}
				n = (base + 1) * y - x;
			}
			seq[i] = 1.0f * n / d;
		}
	}
	AnthemHaltonSequence::AnthemHaltonSequence(uint32_t base, uint32_t size) {
		this->base = base;
		this->size = size;
		this->seq.reserve(size);
		this->seq.resize(size);
		this->generate();
		static_assert(std::weakly_incrementable<std::vector<int>::iterator>);
	}
	float AnthemHaltonSequence::operator[](const uint32_t index) const {
		return this->seq[index];
	}
	AnthemHaltonSequence::iterator AnthemHaltonSequence::begin() {
		return iterator(this->seq.begin());
	}
	AnthemHaltonSequence::iterator AnthemHaltonSequence::end() {
		return iterator(this->seq.end());
	}
}