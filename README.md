# Overview
- This tool is specifically intended to be used in conjunction with a packet capture server to periodically analyze large amounts of packet capture data to extract metadata.
- The overall premise is based on the pcap_feature_extractor with some changes to output things like connection hashes and relevant packet metadata.
- Eventually this tool shall integrate with ZMQ to pump data to a secondary processing tool that will populate a database with data.

# Pages
- [Architecture](./docs/Architecture.md)

# Prerequisites
```bash
apt-get install pcre pcre++ libssl-dev libzmq5-dev cmake libcapstone-dev
python3 -m pip install --upgrade zmq mysql numpy pandas matplotlib boto3 awscli
aws configure
```

