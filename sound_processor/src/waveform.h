#pragma once
 
#include <cstddef>
#include <cstdint>
#include <vector>
 
class Waveform {
public:
  Waveform() = default;
 
  explicit Waveform(size_t sample_count);
 
  Waveform(size_t sample_count, uint32_t sample_rate, uint16_t num_channels,
           uint16_t bits_per_sample);
 
  uint32_t get_sample_rate() const { return sample_rate_; }
  uint16_t get_num_channels() const { return num_channels_; }
  uint16_t get_bits_per_sample() const { return bits_per_sample_; }
 
  void set_meta_info(uint32_t sample_rate, uint16_t num_channels,
                     uint16_t bits_per_sample) {
    sample_rate_ = sample_rate;
    num_channels_ = num_channels;
    bits_per_sample_ = bits_per_sample;
  }
 
  size_t get_sample_count() const { return samples_.size(); }
 
  const std::vector<int16_t> &get_samples() const { return samples_; }
 
  std::vector<int16_t> &get_samples() { return samples_; }
 
  int16_t get_sample_at(size_t index) const;
  void set_sample_at(size_t index, int16_t value);
 
  double get_duration_seconds() const;
 
  size_t seconds_to_samples(double seconds) const;
 
  double samples_index_to_seconds(size_t sample_index) const;
 
  void resize(size_t new_sample_count);
 
  void clear();
 
private:
  uint32_t sample_rate_ = 44100;
  uint16_t num_channels_ = 1;
  uint16_t bits_per_sample_ = 16;
 
  std::vector<int16_t> samples_;
};
