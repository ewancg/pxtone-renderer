#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <vector>

#include "pxtone/pxtnService.h"
#include "sndfile.h"

#define SAMPLE_RATE 48000
#define CHANNEL_COUNT 2

// clang-format off
static const char *usage =
    "Usage: pxtone-decoder [options] file(s)...\n"
    "By default, the provided files will be rendered as a .WAV to a file of the same name.\n"
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
  static std::string encoder(std::string err) { return "encoder: " + err; }
  static std::string encoder(SNDFILE *err) {
    return encoder(std::string(sf_strerror(err)));
  }
};

struct Config {
  enum Format {
    OGG = SF_FORMAT_OGG | SF_FORMAT_VORBIS,
    WAV = SF_FORMAT_WAV | SF_FORMAT_PCM_16,
    FLAC = SF_FORMAT_FLAC | SF_FORMAT_PCM_16
  };
  Format format = OGG;
  std::string formatSuffix = "wav";
  double fadeOutTime = 0;
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
    str += "Warning: ";
  else
    str += "Info: ";

  str += text;
  if (warning == Error) {
    if (!text.empty()) {
      if (*str.end() != '\n') str.push_back('\n');
    }
    str += usage;
  }
  std::cout << str << std::endl;
  return false;  // returns so that errors can be one-liners (return
                 // logToConsole(...))
}

typedef std::pair<std::string, std::string> Arg;
struct KnownArg {
  bool empty() {
    for (auto it : keyMatches)
      if (!it.empty()) return false;
    return true;
  }
  std::set<std::string> keyMatches;
  bool takesParameter = false;
};

static const KnownArg argHelp = {{"--help", "-h"}}, argStdout = {{"--stdout"}},
                      argFormat = {{"--format", "-f"}, true};
static const std::vector<KnownArg> knownArguments = {argHelp, argStdout,
                                                     argFormat};

KnownArg findArgument(const std::string &key) {
  KnownArg match;
  for (auto it : knownArguments) {
    auto arg = it.keyMatches.find(key);
    if (arg != it.keyMatches.end()) {
      match = it;
    }
  }
  return match;
}

static std::set<std::filesystem::path> files;
bool parseArguments(const std::vector<std::string> &args) {
  //  for (auto it : args) std::cout << "'" << it << "'" << std::endl;
  std::map<std::string, std::string> argData;

  bool waitingForSecond = true;
  for (auto it = args.begin(); it != args.end(); ++it) {
    Arg arg;
    while (waitingForSecond) {
      if (arg.first.empty()) {
        if (*it->begin() == '-') {
          auto match = findArgument(*it);
          if (!match.empty()) {
            arg.first = *it;
            waitingForSecond = match.takesParameter;
          } else
            return logToConsole("Unknown argument '" + *it + "'");
        } else {
          files.insert(*it);
          waitingForSecond = false;
        }
      } else {
        if (it != args.end()) ++it;
        if (*it->begin() == '-')
          return logToConsole("Argument '" + arg.first +
                              "' requires a parameter.");
        else {
          arg.second = *it;
          waitingForSecond = false;
        }
      }
    }
    waitingForSecond = true;
    argData.insert(arg);
  }

  for (auto it : argHelp.keyMatches) {
    auto helpFound = argData.find(it);
    if (helpFound == argData.end())
      continue;
    else
      return logToConsole();
  }

  for (auto it : argStdout.keyMatches) {
    auto stdoutFound = argData.find(it);
    if (stdoutFound == argData.end())
      continue;
    else {
      if (files.size() > 1)
        return logToConsole(
            "Standard output cannot be used when rendering multiple files.");
      else
        config.toStdout = true;
    }
  }

  for (auto it : argFormat.keyMatches) {
    auto formatFound = argData.find(it);
    if (formatFound == argData.end() || formatFound->second.empty())
      continue;
    else {
      auto str = formatFound->second;
      std::transform(str.begin(), str.end(), str.begin(),
                     [](unsigned char c) { return std::tolower(c); });
      auto setFormat = [](Config::Format format, const char *suffix) {
        config.format = format;
        config.formatSuffix = suffix;
      };
      str == "ogg"   ? setFormat(Config::OGG, "ogg")
      : str == "wav" ? setFormat(Config::WAV, "wav")
      : str == "flac"
          ? setFormat(Config::FLAC, "flac")
          : void(logToConsole("Unknown format type '" + formatFound->second +
                                  "'; Resorting to .wav",
                              LogState::Warning));
    }
  }
  return true;
}

void decode(std::filesystem::path file) {
  FILE *fp = fopen(file.string().c_str(), "rb");
  if (fp == nullptr)
    throw GetError::file("Error opening file " + file.string() +
                         ". The file may not be readable to your user.");
  fseek(fp, 0L, SEEK_END);
  size_t size = static_cast<size_t>(ftell(fp));
  unsigned char *data = static_cast<unsigned char *>(malloc(size));
  if (data == nullptr)
    throw GetError::generic("Error allocating " + std::to_string(size) +
                            " bytes.");
  rewind(fp);
  if (fread(data, 1, size, fp) != size)
    throw GetError::file("Bytes read does not match file size.");
  fclose(fp);

  pxtnService *pxtn = new pxtnService();

  auto err = pxtn->init();
  if (err != pxtnOK) throw GetError::pxtone(err);
  if (!pxtn->set_destination_quality(CHANNEL_COUNT, SAMPLE_RATE))
    throw GetError::pxtone(
        "Could not set destination quality: " + std::to_string(CHANNEL_COUNT) +
        " channels, " + std::to_string(SAMPLE_RATE) + "Hz.");

  pxtnDescriptor desc;
  if (!desc.set_memory_r(static_cast<void *>(data), static_cast<int>(size)))
    throw GetError::pxtone("Could not set pxtone memory blob of size " +
                           std::to_string(size));
  err = pxtn->read(&desc);
  if (err != pxtnOK) throw GetError::pxtone(err);
  err = pxtn->tones_ready();
  if (err != pxtnOK) throw GetError::pxtone(err);

  pxtnVOMITPREPARATION prep;
  prep.flags |= pxtnVOMITPREPFLAG_loop;  // TODO: figure this out
  prep.start_pos_sample = 0;
  prep.master_volume = 0.8f;  // this is probably good
  prep.fadein_sec = 0;

  if (!pxtn->moo_preparation(&prep))
    throw GetError::pxtone("I Have No Mouth, and I Must Moo");

  //  logToConsole("Successfully opened file " + file.filename().string() + ", "
  //  +
  //                   std::to_string(size) + " bytes read.",
  //               LogState::Info);

  SF_INFO info;
  info.samplerate = SAMPLE_RATE;
  info.channels = CHANNEL_COUNT;
  info.format = config.format;

  if (!sf_format_check(&info))
    throw GetError::encoder("Invalid encoder format.");

  SNDFILE *pcmFile =
      sf_open(std::filesystem::absolute(
                  file.replace_extension(config.formatSuffix).filename())
                  .string()
                  .c_str(),
              SFM_WRITE, &info);
  if (pcmFile == nullptr) throw GetError::encoder(file);

  int seconds = pxtn->master->get_meas_num() * pxtn->master->get_beat_num() /
                pxtn->master->get_beat_tempo() * 60;

  int num_samples = int(SAMPLE_RATE * seconds);
  int renderSize = num_samples * CHANNEL_COUNT * 16 / 8;

  int written = 0;
  char *buf = static_cast<char *>(malloc(static_cast<size_t>(renderSize)));
  auto render = [&](int len) {
    while (written < len) {
      int mooed_len = 0;
      if (!pxtn->Moo(buf, len - written, &mooed_len))
        throw "Moo error during rendering. Bytes written so far: " +
            std::to_string(written);

      written += mooed_len;
    }
  };
  render(renderSize);

  sf_write_raw(pcmFile, buf, renderSize);
  //  logToConsole("Successfully wrote " + std::to_string(renderSize) + "
  //  bytes",
  //               LogState::Info);

  sf_set_string(pcmFile, SF_STR_TITLE, pxtn->text->get_name_buf(nullptr));
  sf_set_string(pcmFile, SF_STR_COMMENT, pxtn->text->get_comment_buf(nullptr));

  sf_close(pcmFile);
  pxtn->evels->Release();
}

int main(int argc, char *argv[]) {
  if (argc <= 1) return logToConsole("At least 1 .ptcop file is required.");

  std::vector<std::string> args;
  for (int i = 1; i < argc; i++) {
    std::string str = argv[i];
    std::replace(str.begin(), str.end(), '=', ' ');
    std::stringstream ss(str);
    while (getline(ss, str, ' ')) args.push_back(str);
  }

  if (!parseArguments(args)) return 0;

  for (auto it : files) {
    auto absolute = std::filesystem::absolute(it);
    if (std::filesystem::exists(it)) try {
        decode(std::filesystem::absolute(it));
      } catch (std::string err) {
        logToConsole(err);
      }
    else
      logToConsole("File " + std::string(it) + " not found.",
                   LogState::Warning);
  }
  return 0;
}
