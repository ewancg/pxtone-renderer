# pxtone-renderer
Render pxtone projects.

```
Usage: pxtone-decoder [options] file(s)...
By default, the provided files will be rendered as .wav to your working directory.
Options:
  --format, -f        [OGG, WAV, FLAC]    Encode data to this format.
  --fadein            [seconds]           Specify song fade in time.
  --loop, -l          Loop the song this many times.
  --loop-separately   Separate the song into 'intro' and 'loop' files.

  --output, -o   If 1 file is being rendered, place the resulting file here.
                 If multiple are being rendered, put them in this directory.

  --help, -h     Show this dialog.
  --quiet, -q    Omit info & warning messages from console output.
```

Requires C++17 and CMake. libogg/vorbis by xiph.org, pxtone by Studio Pixel, bugfixes by Clownacy

on msys please use Unix Makefiles (-G)
