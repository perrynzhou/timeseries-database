#!/bin/bash
make clean
make test_conf
make test_skip
./test_skip
