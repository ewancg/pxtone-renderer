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
    "By default, the provided files will be rendered as .wav to your working directory.\n"
    "Options:\n"
    "  --format, -f           [OGG, WAV, FLAC]    Encode data to this format.\n"
    "  --vbr, -v              [0.0 - 1.0]         FLAC/OGG only; Set VBR quality.\n"
    "  --compression, -c      [0.0 - 1.0]         FLAC/OGG only; Set compression level.\n"
    "  --fadeout              [seconds]           Specify song fadeout time.\n"
    "\n"
//    "  --stdout       Single-file only: Use standard output instead of a file.\n" // mehhhhh i don't wanna implement virtual file i/o
    "  --output, -o   Single-file only: Render to this file instead of your working directory.\n"
    "\n"
    "  --help, -h     Show this dialog.\n"
    "  --quiet, -q    Omit info & warning messages from console output.\n"
;
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
  double fadeOutTime = 0, vbrRate = 0, compressionRate = 0;
  //  bool toStdout = false;
  bool quiet = true;
  std::string filePath;
} static config;

enum LogState : unsigned char { Error, Warning, Info };
bool help() {
  std::cout << usage << std::endl;
  return false;
}
bool logToConsole(std::string text, LogState warning = Error) {
  //  if (config.toStdout) return false;
  std::string str;
  if (warning == Error)
    str += "Error: ";
  else if (warning == Warning)
    str += "Warning: ";
  else
    str += "Info: ";

  str += text;
  if (*str.end() != '\n') str.push_back('\n');
  if (warning != Error) {
    if (!config.quiet) std::cout << str << std::endl;
  } else {
    std::cout << str << std::endl;
    help();
  }
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

static const KnownArg argFormat = {{"--format", "-f"}, true},
                      argVbr{{"--vbr", "-v"}, true},
                      argCompression{{"--compression", "-c"}, true},
                      //                      argStdout = {{"--stdout"}},
    argOutput = {{"--output", "-o"}, true}, argHelp = {{"--help", "-h"}},
                      argQuiet{{"--quiet", "-q"}},
                      argFadeOut{{"--fadeout"}, true};

static const std::vector<KnownArg> knownArguments = {
    argFormat, argVbr,   argCompression, /*argStdout,*/ argOutput,
    argHelp,   argQuiet, argFadeOut};

KnownArg findArgument(const std::string &key) {
  KnownArg match;
  for (auto it : knownArguments) {
    auto arg = it.keyMatches.find(key);
    if (arg != it.keyMatches.end()) match = it;
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
        bool argSupplied = true;
        if (++it != args.end()) {
          if (*it->begin() == '-')
            argSupplied = false;
          else {
            arg.second = *it;
            waitingForSecond = false;
          }
        } else
          argSupplied = false;
        if (!argSupplied)
          return logToConsole("Argument '" + arg.first +
                              "' requires a parameter.");
      }
    }
    waitingForSecond = true;
    argData.insert(arg);
  }

  for (auto it : argHelp.keyMatches) {
    auto helpFound = argData.find(it);
    if (helpFound != argData.end()) return help();
  }

  for (auto it : argFadeOut.keyMatches) {
    auto fadeOutFound = argData.find(it);
    if (fadeOutFound != argData.end())
      config.fadeOutTime = std::stod(fadeOutFound->second);
  }

  for (auto it : argQuiet.keyMatches) {
    auto quietFound = argData.find(it);
    if (quietFound != argData.end()) config.quiet = false;
  }

  for (auto it : argVbr.keyMatches) {
    auto vbrFound = argData.find(it);
    if (vbrFound != argData.end()) config.vbrRate = std::stod(vbrFound->second);
  }
  for (auto it : argCompression.keyMatches) {
    auto compressionFound = argData.find(it);
    if (compressionFound != argData.end())
      config.compressionRate = std::stod(compressionFound->second);
  }

  for (auto it : argOutput.keyMatches) {
    auto outputFound = argData.find(it);
    if (outputFound != argData.end()) {
      if (files.size() > 1)
        return logToConsole(
            "Output file path can only be used when processing 1 file.");  // Potentially
                                                                           // replace
                                                                           // with
                                                                           // output
                                                                           // directory
                                                                           // mode
                                                                           // instead
      else
        config.filePath = outputFound->second;
    }
  }

  //  for (auto it : argStdout.keyMatches) {
  //    auto stdoutFound = argData.find(it);
  //    if (stdoutFound != argData.end()) {
  //      if (files.size() > 1)
  //        return logToConsole(
  //            "Standard output cannot be used when rendering multiple
  //            files.");
  //      else
  //        config.toStdout = true;
  //    }
  //  }

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

void convert(std::filesystem::path file) {
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
  prep.fadein_sec = config.fadeOutTime;

  if (!pxtn->moo_preparation(&prep))
    throw GetError::pxtone("I Have No Mouth, and I Must Moo");

  logToConsole("Successfully opened file " + file.filename().string() + ", " +
                   std::to_string(size) + " bytes read.",
               LogState::Info);

  SF_INFO info;
  info.samplerate = SAMPLE_RATE;
  info.channels = CHANNEL_COUNT;
  info.format = config.format;

  if (!sf_format_check(&info))
    throw GetError::encoder("Invalid encoder format.");

  std::filesystem::path newFilePath =
      config.filePath.empty()
          ? std::filesystem::absolute(
                file.replace_extension(config.formatSuffix).filename())
          : std::filesystem::absolute(config.filePath);

  SNDFILE *pcmFile = sf_open(newFilePath.string().c_str(), SFM_WRITE, &info);

  if (pcmFile == nullptr) throw GetError::encoder(pcmFile);

  sf_command(pcmFile, SFC_SET_COMPRESSION_LEVEL, &config.compressionRate,
             sizeof(double));
  sf_command(pcmFile, SFC_SET_VBR_ENCODING_QUALITY, &config.vbrRate,
             sizeof(double));

  int seconds = pxtn->master->get_meas_num() * pxtn->master->get_beat_num() /
                pxtn->master->get_beat_tempo() * 60;

  int num_samples = int(SAMPLE_RATE * seconds);
  int renderSize = num_samples * CHANNEL_COUNT * 16 / 8;

  int written = 0;
  char *buf = static_cast<char *>(malloc(static_cast<size_t>(renderSize + 1)));
  buf[renderSize + 1] = '\0';
  auto render = [&](int len) {
    while (written < len) {
      int mooedLength = 0;
      if (!pxtn->Moo(buf, len - written, &mooedLength))
        throw "Moo error during rendering. Bytes written so far: " +
            std::to_string(written);

      written += mooedLength;
    }
  };
  render(renderSize);

  sf_set_string(pcmFile, SF_STR_TITLE, pxtn->text->get_name_buf(nullptr));
  sf_set_string(pcmFile, SF_STR_COMMENT, pxtn->text->get_comment_buf(nullptr));
  sf_command(pcmFile, SFC_UPDATE_HEADER_NOW, nullptr, 0);

  if (int size = sf_write_raw(pcmFile, buf, renderSize) != renderSize)
    throw GetError::file("Error writing complete audio buffer: wrote " +
                         std::to_string(size) + " out of " +
                         std::to_string(renderSize));
  // i'm not supposed to use write_raw; even though it works i think it
  // might only be for wav, ogg/flac don't support? write ints instead:
  // custom logic below

  //  int halved = renderSize / 2;
  //  //  short *z[halved];  //= static_cast<short
  //  **>(malloc(renderSize)); for (int i = 0; i < halved; i++) {
  //    short s = (buf)[i * 2] << 8 | (buf)[(i * 2) + 1];
  //    //    short s = static_cast<short>(buf[i * 2]);
  //    sf_writef_short(pcmFile, &s, 1 /*halved*/);
  //    //    z[i] = &s;
  //  }
  // this code does not work right now; when it does work it creates empty files

  sf_write_sync(pcmFile);
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
        convert(std::filesystem::absolute(it));
      } catch (std::string err) {
        logToConsole(err);
      }
    else
      logToConsole("File " + it.string() + " not found.", LogState::Warning);
  }
  return 0;
}

/* TODO:
 *  figure out why it makes empty files -- i think it is not mooing
 *
 *  make it do intro & loop sections separately
 *
 *  clean up cmakelists and add deployment for libraries locally instead of
 *  cmake-dependent
 *
 *  fix changes in spaces in filename - oversight when doin getline
 *
 *  implement & test other formats libsndfile supports (big ones are opus and
 *  mp3)
 **/
