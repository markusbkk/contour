#! /bin/bash
set -ex
sudo apt -q update
sudo apt install -y \
            libqt5core5a \
            libqt5gui5 \
            libqt5network5 \
            libqt5qml5 \
            \
            xvfb \
            \
            ffmpeg \
            libavcodec58 \
            libavformat58 \
            libavutil56 \
            libdeflate0 \
            libncurses6 \
            libqrcodegen1 \
            libswscale5 \
            libunistring2 \
            \
            libfontconfig1 \
            libfreetype6 \
            libharfbuzz0b \
            libqt5core5a \
            libqt5gui5 \
            libqt5network5 \
            libqt5multimedia5 \
            libqt5x11extras5 \
            libyaml-cpp0.7
