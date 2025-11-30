FROM ubuntu:24.04 AS sdk_setup

# Pico-SDK args
ENV PICO_SDK_VERSION=2.2.0
ARG SDK_DIR=/home/pico-sdk

# Pico-SDK tools args
ARG PICO_SDK_TOOLS_DIR=/home/pico-sdk-tools

# Picotool args
ARG PICOTOOL_DIR=/home/picotool

RUN set -x \
    && apt-get update -y \
    && apt-get upgrade -y

RUN set -x \
    && apt-get install --no-install-recommends -y \
        git \
        openssh-client \
        ca-certificates \
        tar \
        jq \
        curl \
        wget \
        usbutils

RUN set -x \
    && apt-get install --no-install-recommends -y \
        python3 \
        clang-format \
        build-essential \
        cmake \
        gcc-arm-none-eabi \
        gdb-multiarch \
        libnewlib-arm-none-eabi \
        libstdc++-arm-none-eabi-newlib

# -------- Pico-SDK --------
RUN set -x \
    && git clone --depth 1 --recurse-submodules --branch $PICO_SDK_VERSION https://github.com/raspberrypi/pico-sdk $SDK_DIR

ENV PICO_SDK_PATH=$SDK_DIR

# -------- Pico-SDK Tools --------
# Download OpenOCD prebuilt binary 
RUN set -x \
    && mkdir -p $PICO_SDK_TOOLS_DIR \
    && OPENOCD_DL_URL=$(curl -sL https://api.github.com/repos/raspberrypi/pico-sdk-tools/releases/latest | jq -r '.assets[] | .browser_download_url' | grep -e openocd.*x86_64-lin.tar.gz) \
    && wget $OPENOCD_DL_URL -P $PICO_SDK_TOOLS_DIR/openocd/ \
    && tar -xzf $PICO_SDK_TOOLS_DIR/openocd/*.tar.gz -C $PICO_SDK_TOOLS_DIR/openocd/ \
    && rm $PICO_SDK_TOOLS_DIR/openocd/*.tar.gz

ENV PICO_SDK_TOOLS_PATH=$PICO_SDK_TOOLS_DIR

# OpenOCD dependencies
RUN set -x \
    && apt-get install --no-install-recommends -y \
        libftdi1-2 \
        libhidapi-hidraw0

# -------- Picotools --------
# Download Picotool latest release
# RUN set -x \
#     && mkdir -p $PICOTOOL_DIR/picotool \
#     && PICOTOOL_DL_URL=$(curl -sL https://api.github.com/repos/raspberrypi/picotool/releases/latest | jq -r '.assets[] | .browser_download_url' | grep -e picotool.*.tar.gz) \
#     && wget $PICOTOOL_DL_URL -P $PICOTOOL_DIR/ \
#     && tar -xzf $PICOTOOL_DIR/*.tar.gz -C $PICOTOOL_DIR/picotool/ --strip-components=1 \
#     && rm $PICOTOOL_DIR/*.tar.gz

# ENV picotool_DIR=$PICOTOOL_DIR

# Clean up apt cache to reduce image size
RUN set -x \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*