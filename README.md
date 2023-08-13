With appreciation to

https://github.com/fxwiegand/terrarium-reverb/


# Setup

1. Install toolchain

```bash
brew install gcc-arm-embedded
brew install dfu-util
```

2. Fetch and make depedencies

```bash
git submodule update --init --recursive
cd libDaisy && make && ..
cd DaisySP && make && ..
cd Terrarium && make && ..
cd basic_reverb && make
```

3. Find device and upload

Hold down reset, hold down boot, let go of reset, let go of boot (while plugged into USB)

```bash
# list devices to find device address and settings
dfu-util --devices
# upload (aka download) to device
dfu-util --device 0483:df11 --alt 0 --download build/reverb.bin --dfuse-address 0x08000000
```
