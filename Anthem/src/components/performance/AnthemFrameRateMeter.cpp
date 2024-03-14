#include "../../../include/components/performance/AnthemFrameRateMeter.h"

namespace Anthem::Components::Performance {
	void AnthemFrameRateMeter::record() {
		auto t = this->getDuration();
		this->accumulatedTime += t;
		timeRecords.push_back(t);
		if (timeRecords.size() > maxRecord) {
			auto f = timeRecords.front();
			timeRecords.pop_front();
			this->accumulatedTime -= f;
		}
		this->recordTime();
	}
	double AnthemFrameRateMeter::getFrameRate() {
		return timeRecords.size() * 1.0 / accumulatedTime * 1000;
	}

	AnthemFrameRateMeter::AnthemFrameRateMeter(int64_t recordSize) {
		this->maxRecord = recordSize;
	}
}