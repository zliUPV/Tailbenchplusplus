#FROM ubuntu:14.04
#MAINTAINER Alexandros Daglis <alexandros.daglis@epfl.ch>

apt-get update -y && apt-get install -y --no-install-recommends \
      build-essential \
    && rm -rf /var/lib/apt/lists/*

cp -r files/* /root/

mkdir /videos
cd /root/filegen && make && ./generate_video_files_and_logs.sh /videos

#VOLUME ["/videos"]

