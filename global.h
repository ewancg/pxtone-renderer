#pragma once

#include <iostream>
#include <string>

#include "pxtone/pxtnError.h"
#include "vorbis/vorbisenc.h"

#define SAMPLE_RATE 48000
#define CHANNEL_COUNT 2

// clang-format off
static const char *usage =
    "Usage: pxtone-decoder [options] file(s)...\n"
    "By default, the provided files will be rendered as a .OGG to a file of the same name.\n"
    "Options:\n"
    "  -f, --format <OGG:WAV:FLAC>  Encode data to this format.\n"
    "  -h, --help                   Show this dialog.\n"
    "  --stdout                     Output the data into stdout (single-file only).\n"
    "  -q, --quiet                  Omit info & warning messages from console output.";
// clang-format on

struct GetError {
  static std::string pxtone(std::string err) { return "pxtone: " + err; }
  static std::string pxtone(pxtnERR err) {
    return pxtone(std::string(pxtnError_get_string(err)));
  }
  static std::string generic(std::string err) { return "generic: " + err; }
  static std::string file(std::string err) { return "file: " + err; }
};

struct Config {
  enum Format : unsigned char { OGG, WAV, FLAC };
  Format format = OGG;
  bool multipleFiles = false;
  bool toStdout = false;
  std::string *filePath;
} static config;

enum LogState : unsigned char { Error, Warning, Info };
bool logToConsole(std::string text = "", LogState warning = Error) {
  std::string str;
  if (warning == Error)
    str += "Error: ";
  else if (warning == Warning)
    str += "Warning";
  else
    str += "Info: ";

  if (!text.empty()) {
    str += text;
    if (*str.end() != '\n') str.push_back('\n');
  }
  if (warning == Error) str += usage;
  std::cout << str << std::endl;
  return false;  // returns so that errors can be one-liners (return
                 // logToConsole(...))
};
