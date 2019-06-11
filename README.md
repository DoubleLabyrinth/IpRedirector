# Simple IP Redirector

A library that can map one IP:Port to another IP:Port.

This project is based on `WinDivert`.

# TODO

- [x] IPv4 redirect

- [ ] IPv6 redirect 

- [ ] Documents

# Demo for IPv4

```
// Require Administrator privilege
Usage:
    Ipv4RedirectorDemo.exe <Redirect from IP> <Redirect from port> <Redirect to IP> <Redirect to port>
```

Example:

```
// Require Administrator privilege
$ Ipv4RedirectorDemo.exe 123.123.123.123 80 127.0.0.1 4444
```

After that, all packets that should be sent to `123.123.123.123:80` will be sent (redirected) to `127.0.0.1:4444`.

If you want to stop, just press `Enter`.

