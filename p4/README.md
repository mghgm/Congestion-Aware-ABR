# P4 codes

## TCP-INT
In this P4 code the TCP-INT is implemented based on [In-band Network Telemetry (INT) Dataplane Specification](https://p4.org/p4-spec/docs/INT_v2_1.pdf). The following options is added/updated on each switch.

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+---------------+---------------+-------+-------+---------------+
|  Kind = 0x72  |  Length = 12  |TagFreq|LinkSpd|    INTval     |
+---------------+---------------+-------+-------+---------------+
|  HopID=IP.TTL |                  HopLat (3B)                  |
+---------------+-------+-------+-------------------------------+
|     INTEcr    |LnkSEcr| HIDEcr|        HopLatEcr (2B)         |
----------------+-------+-------+-------------------------------+
```

- `HopID`: ID of most congested switch.
- `HopLat`: Timedelta in the most congested switch.
- `HopLatEcr`: Qeueu length in the most congested hop.
- `INTEcr`: Number congested hops in the path. Identifed by `QUEUE_DEPTH_TH`.

