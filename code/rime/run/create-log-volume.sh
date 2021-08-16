#!/bin/bash

set -ex

case $1 in
  /*) is_full_path=true ;;
   *) echo "Not a full path" ;;
esac

if [ $is_full_path ];
then
  sudo docker volume create --driver local \
  --opt device=$1 \
  --opt type=ext4 \
  --name rime_logs
fi