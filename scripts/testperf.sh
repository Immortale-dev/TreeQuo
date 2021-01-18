#!/bin/bash

set -e

make perf
./perf_test.exe
