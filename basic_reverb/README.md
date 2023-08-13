
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

```bash
# list devices to find device address and settings
dfu-util --devices
# upload (aka download) to device
dfu-util --device 0483:df11 --alt 0 --download build/reverb.bin --dfuse-address 0x08000000
```
