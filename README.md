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

## Requirements
 - C++17
 - CMake
 - Ogg
 - Vorbis
 - FLAC
 - libsndfile

No C++ bindings required for any of the libraries.

For convenience, all of the libraries are available as git submodules in the `extern` directory.

## Building
CMake must be able to find all the libraries within its search paths.
After the prerequisites are met, the following should be fine:

`cmake -B ./build -S ./ && cmake --build ./build`

## Thanks
 - OPNA2608; big endian fixes, CI, testing 
 - Sidedishes; testing
 - Clownacy; bugfixes in pxtone, testing
 - SlightlyIntelligentMonkey; testing
 - Xiph.org; Ogg, Vorbis and FLAC
 - Studio Pixel; pxtone
 - libsndfile guys


