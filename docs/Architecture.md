# Architecture
- All packets shall be assigned to a connection based on various parameters.
- All connections shall be hashed based on source, destination, timestamp, and TCP/UDP src/dst port information.
- ICMP/IPSEC/other packets shall be treated as non-connections (for now).

