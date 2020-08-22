#!/bin/bash
protoc -I=../src/protobuf --python_out=. ../src/protobuf/Messages.proto
