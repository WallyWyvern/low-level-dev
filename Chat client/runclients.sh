#!/usr/bin/env bash
for run in {1..10}; do
  urxvt -e cmake-build-debug/src/chat_client &
done
