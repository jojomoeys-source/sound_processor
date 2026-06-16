#include "waveform/waveform.h"
#include <cmath>
#include <limits>
#include <stdexcept>

Waveform::Waveform(size_t sample_count) : samples_(sample_count) {}

Waveform::Waveform(size_t sample_count, uint32_t sample_rate,
                   uint16_t num_channels, uint16_t bits_per_sample)
    : sample_rate_(sample_rate), num_channels_(num_channels),
      bits_per_sample_(bits_per_sample), samples_(sample_count) {}

int16_t Waveform::get_sample_at(size_t index) const {
  return samples_.at(index);
}

void Waveform::set_sample_at(size_t index, int16_t value) {
  samples_.at(index) = value;
}

double Waveform::get_duration_seconds() const {
  if (sample_rate_ == 0) {
    throw std::runtime_error("Sample rate cannot be zero.");
  }
  return static_cast<double>(samples_.size()) /
         static_cast<double>(sample_rate_);
}

size_t Waveform::seconds_to_samples(double seconds) const {
  if (seconds < 0.0) {
    throw std::invalid_argument("Seconds cannot be negative.");
  }
  const double sample_count = seconds * static_cast<double>(sample_rate_);
  if (!std::isfinite(sample_count) ||
      sample_count > static_cast<double>(std::numeric_limits<size_t>::max())) {
    throw std::overflow_error("Seconds value is too large.");
  }
  return static_cast<size_t>(std::round(sample_count));
}

double Waveform::samples_index_to_seconds(size_t sample_index) const {
  if (sample_rate_ == 0) {
    throw std::runtime_error("Sample rate cannot be zero.");
  }
  return static_cast<double>(sample_index) / static_cast<double>(sample_rate_);
}

void Waveform::resize(size_t new_sample_count) {
  samples_.resize(new_sample_count);
}

void Waveform::clear() {
  *this = Waveform{};
}
