#!/usr/bin/env bash

make -j 2
make rename
xvfb-run make check -j 2