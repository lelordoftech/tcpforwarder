# TCP Forwarder

## 1. How to build:

```cpp
g++ tcpForwarder.cpp -lpthread -o tcpForward
```

## 2. How to use:

### a. Without parameter, it will use the default setting:

* input: 192.168.1.100:44405
* output: 192.168.1.100:44406

```
./tcpForward
```

### a. With parameter, it will use this setting:

* input: ip_in:port_in
* output: ip_out:port_out

```
./tcpForward ip_in port_in ip_out port_out
```
