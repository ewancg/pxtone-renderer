#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <vector>

#include "pxtone/pxtnService.h"
#include "sndfile.h"

#pragma pack(1)
#define SAMPLE_RATE 48000
#define CHANNEL_COUNT 2

// clang-format off
constexpr char usage[] =
    "Usage: pxtone-decoder [options] file(s)...\n"
    "By default, the provided files will be rendered as .wav to your working directory.\n"
    "Options:\n"
    "  --format, -f        [OGG, WAV, FLAC]    Encode data to this format.\n"
    "  --vbr, -v           [0.0 - 1.0]         FLAC/OGG only; Set VBR quality.\n"
    "  --compression, -c   [0.0 - 1.0]         FLAC/OGG only; Set compression level.\n"
    "  --fadeout           [seconds]           Specify song fadeout time.\n"
    "  --loop, -l          Loop the song this many times.\n"
    "  --loop-separately   Separate the song into 'intro' and 'loop' files. Can't be used with -l.\n"
    "\n"
    "  --output, -o   If 1 file is being rendered, place the resulting file here.\n"
    "                 If multiple are being rendered, put them in this directory.\n"
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
  Format format = WAV;
  //  bool toStdout = false;
  int loopCount = 1;
  bool loopSeparately = false, quiet = true, singleFile = true,
       outputToDirectory = false;
  double fadeOutTime = 0, vbrRate = 0, compressionRate = 0;
  std::string formatSuffix = "wav", fileName;
  std::filesystem::path outputDirectory;
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
  if (warning != Error && config.quiet)
    return false;
  else {
    std::cout << str << std::endl;
    if (warning == Error) help();
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
                      argFadeOut{{"--fadeout"}, true},
                      argLoop{{"--loop", "-l"}, true},
                      argLoopSeparately{{"--loop-separately"}};

static const std::vector<KnownArg> knownArguments = {
    argFormat, argVbr,     argCompression, /*argStdout,*/ argOutput, argHelp,
    argQuiet,  argFadeOut, argLoop,        argLoopSeparately};

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
  switch (files.size()) {
    case 0:
      return logToConsole("At least 1 .ptcop file is required.");
    case 1:
      config.singleFile = true;
      break;
    default:
      config.singleFile = false;
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
    if (quietFound == argData.end()) config.quiet = false;
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
  for (auto it : argLoopSeparately.keyMatches) {
    auto loopSeparatelyFound = argData.find(it);
    if (loopSeparatelyFound != argData.end()) config.loopSeparately = true;
  }
  for (auto it : argLoop.keyMatches) {
    auto loopFound = argData.find(it);
    if (loopFound != argData.end()) {
      if (config.loopSeparately)
        return logToConsole(
            "A loop count can't be specified when rendering the loop "
            "separately.");
      if (config.loopSeparately)
        config.loopCount = std::abs(std::stoi(loopFound->second));
    }
  }

  std::filesystem::path path =
      std::filesystem::absolute(std::filesystem::current_path());
  for (auto it : argOutput.keyMatches) {
    auto outputFound = argData.find(it);
    if (outputFound != argData.end())
      path = std::filesystem::absolute(outputFound->second);
  }

  config.outputToDirectory = std::filesystem::is_directory(path);
  if (config.singleFile && !config.outputToDirectory)
    config.fileName = path.filename().string();
  config.outputDirectory = std::filesystem::absolute(path);

  //  for (auto it : argStdout.keyMatches) {
  //    auto stdoutFound = argData.find(it);
  //    if (stdoutFound != argData.end()) {
  //      if (!config.singleFile)
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

  //  pxtn->text->Comment_r(&desc); // not working rn ig
  //  pxtn->text->Name_r(&desc);
  //  pxtnVOMITPREPARATION prep;
  //  prep.flags |= pxtnVOMITPREPFLAG_loop;  // TODO: figure this out
  //  prep.start_pos_sample = 0;
  //  prep.master_volume = 0.8f;  // this is probably good
  //  prep.fadein_sec = static_cast<float>(config.fadeOutTime);

  //  if (!pxtn->moo_preparation(&prep))
  //    throw GetError::pxtone("I Have No Mouth, and I Must Moo");

  SF_INFO info;
  info.samplerate = SAMPLE_RATE;
  info.channels = CHANNEL_COUNT;
  info.format = config.format;

  if (!sf_format_check(&info))
    throw GetError::encoder("Invalid encoder format.");

  std::filesystem::path introPath = config.outputDirectory;

  if (config.outputToDirectory) {
    if (!std::filesystem::exists(introPath))
      if (!std::filesystem::create_directory(introPath))
        throw GetError::file("Unable to create destination path.");
    if (config.singleFile && !config.fileName.empty())
      introPath += "/" + config.fileName;
    else
      introPath +=
          "/" + file.filename().replace_extension(config.formatSuffix).string();
  }

  auto render = [&info, &pxtn](int measureCount, int startMeas,
                               std::string path, bool loop) {
    int sampleCount =
        SAMPLE_RATE * (measureCount * pxtn->master->get_beat_num() /
                       pxtn->master->get_beat_tempo() * 60);
    int renderSize = sampleCount * CHANNEL_COUNT * 16 / 8;

    pxtnVOMITPREPARATION prep = {};
    prep.flags |= pxtnVOMITPREPFLAG_loop;  // TODO: figure this out
    prep.start_pos_meas = startMeas;
    prep.master_volume = 0.8f;  // this is probably good
    prep.fadein_sec = static_cast<float>(config.fadeOutTime);

    if (!pxtn->moo_preparation(&prep))
      throw GetError::pxtone("I Have No Mouth, and I Must Moo");

    SNDFILE *pcmFile = sf_open(path.c_str(), SFM_WRITE, &info);

    if (pcmFile == nullptr) throw GetError::encoder(pcmFile);

    sf_command(pcmFile, SFC_SET_COMPRESSION_LEVEL, &config.compressionRate,
               sizeof(double));
    sf_command(pcmFile, SFC_SET_VBR_ENCODING_QUALITY, &config.vbrRate,
               sizeof(double));

    sf_command(pcmFile, SFC_UPDATE_HEADER_NOW, nullptr, 0);

    char *buf =
        static_cast<char *>(malloc(static_cast<size_t>(renderSize + 1)));
    buf[renderSize + 1] = '\0';

    int written = 0;
    auto mooSection = [&](int len) {
      while (written < len) {
        int mooedLength = 0;
        if (!pxtn->Moo(buf, len - written, &mooedLength))
          throw "Moo error during rendering. Bytes written so far: " +
              std::to_string(written);

        written += mooedLength;
      }
    };

    int loopCount = loop ? config.loopCount : 1;
    for (int i = loopCount; i > 0; i--) {
      mooSection(renderSize);
      short *shorts = reinterpret_cast<short *>(buf);
      sf_write_short(pcmFile, shorts, renderSize / 2);
    }

    sf_set_string(pcmFile, SF_STR_TITLE, pxtn->text->get_name_buf(nullptr));
    sf_set_string(pcmFile, SF_STR_COMMENT,
                  pxtn->text->get_comment_buf(nullptr));

    sf_write_sync(pcmFile);
    sf_close(pcmFile);
  };

  int lastMeasure = pxtn->master->get_meas_num();
  if (pxtn->master->get_last_meas())
    lastMeasure = pxtn->master->get_last_meas();

  if (config.loopSeparately && pxtn->master->get_repeat_meas() > 0) {
    std::filesystem::path loopPath = introPath;
    loopPath.replace_filename(loopPath.filename().stem().string() + "_loop" +
                              loopPath.extension().string());
    introPath.replace_filename(introPath.filename().stem().string() + "_intro" +
                               introPath.extension().string());

    render(pxtn->master->get_repeat_meas(), 0, introPath.string(), false);
    render((lastMeasure - pxtn->master->get_repeat_meas()),
           pxtn->master->get_repeat_meas(), loopPath.string(), true);
  } else {
    if (config.loopSeparately && pxtn->master->get_repeat_meas() == 0)
      logToConsole(file.filename().string() +
                       " does not have a loop point. It will be "
                       "rendered to one file.",
                   LogState::Warning);
    render(lastMeasure, 0, introPath.string(), true);
  }
  //  render(pxtn->master->get_repeat_meas() * pxtn->master->get_beat_num() /
  //             pxtn->master->get_beat_tempo() * 60,
  //         introPath.string());

  //  int written = 0;
  //  char *buf = static_cast<char *>(malloc(static_cast<size_t>(renderSize +
  //  1))); buf[renderSize + 1] = '\0'; auto render = [&](int len) {
  //    while (written < len) {
  //      int mooedLength = 0;
  //      if (!pxtn->Moo(buf, len - written, &mooedLength))
  //        throw "Moo error during rendering. Bytes written so far: " +
  //            std::to_string(written);

  //      written += mooedLength;
  //    }
  //  };
  //  render(renderSize);

  //  sf_set_string(pcmFile, SF_STR_TITLE, pxtn->text->get_name_buf(nullptr));
  //  sf_set_string(pcmFile, SF_STR_COMMENT,
  //  pxtn->text->get_comment_buf(nullptr)); sf_command(pcmFile,
  //  SFC_UPDATE_HEADER_NOW, nullptr, 0);

  //  if (int size = sf_write_raw(pcmFile, buf, renderSize) != renderSize)
  //    throw GetError::file("Error writing complete audio buffer: wrote " +
  //                         std::to_string(size) + " out of " +
  //                         std::to_string(renderSize));
  // i'm not supposed to use write_raw; even though it works i think it
  // might only be for wav, ogg/flac don't support? write ints instead:
  // custom logic below

  //  int halved = renderSize / 2;
  //  short *z[halved];
  //  = static_cast<short **>(malloc(renderSize));
  //  for (int i = 0; i < halved; i++) {
  //    short s = (buf)[i * 2] << 8 | (buf)[(i * 2) + 1];
  //    //    short s = static_cast<short>(buf[i * 2]);
  //    sf_writef_short(pcmFile, &s, 1 /*halved*/);
  //    //    z[i] = &s;
  //  }
  // this code does not work right now; when it does work it creates empty files

  pxtn->evels->Release();
}

int main(int argc, char *argv[]) {
  if (argc <= 1) return logToConsole("Expected at least 1 parameter.");

  std::vector<std::string> args;
  //  for (int i = 1; i < argc; i++) {
  //    std::string str = argv[i];
  //    std::replace(str.begin(), str.end(), '=', ' ');
  //    std::stringstream ss(str);
  //    while (getline(ss, str, ' ')) args.push_back(str);
  //  }

  std::string str;
  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    auto parseArg = [&]() -> bool {
      size_t x = arg.find('=');
      if (x != arg.npos) {
        args.push_back(arg.substr(0, x));
        arg.erase(0, x + 1);
        args.push_back(arg);
        return arg.find('=') != arg.npos;
      } else {
        args.push_back(arg);
        return false;
      }
    };
    if (parseArg()) parseArg();
  }

  if (!parseArguments(args)) return 0;

  for (auto it : files) {
    auto absolute = std::filesystem::absolute(it);
    if (std::filesystem::exists(it)) try {
        convert(std::filesystem::absolute(it));
      } catch (std::string err) {
        return logToConsole(err);
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
 *  add loop count
 *
 *  remove arg parsing boilerplate
 *
 *  clean up cmakelists and add deployment for libraries locally instead of
 *  cmake-dependent
 *
 *  implement & test other formats libsndfile supports (big ones are opus and
 *  mp3)
 **/
