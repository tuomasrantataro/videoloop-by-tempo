#!/bin/sh

packages="libqt5widgets5 libqt5multimedia5 libqt5multimedia5-plugins"

apt install $packages

cp ./lib/libessentia.so /usr/lib