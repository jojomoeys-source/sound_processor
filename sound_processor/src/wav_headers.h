#pragma once
#include <cstdint>
 
#pragma pack(push, 1)
 
struct RiffHeader {
  char chunk_id[4];
  uint32_t chunk_size;
  char format[4];
};
 
struct FmtHeader {
  char subchunk_id[4];
  uint32_t subchunk_size;
  uint16_t audio_format;
  uint16_t num_channels;
  uint32_t sample_rate;
  uint32_t byte_rate;
  uint16_t block_align;
  uint16_t bits_per_sample;
};
 
struct DataHeader {
  char subchunk_id[4];
  uint32_t subchunk_size;
};
 
#pragma pack(pop)