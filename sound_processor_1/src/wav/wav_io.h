#pragma once

#include "waveform/waveform.h"
#include <string>

class WavReader {
public:
  static Waveform read(const std::string &file_path);
};

class WavWriter {
public:
  static void write(const std::string &file_path, const Waveform &waveform);
};