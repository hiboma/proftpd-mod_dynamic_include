# mod_dynamic_include.c

## Description

Load proftpd configuration dynamically on Login phase

## Usage

/etc/proftpd.conf

```
LoadModule mod_dynamic_include.c
DynamicIncludePath /etc/proftpd/%u.conf
```

## format spec

```
%u               user name ( sent by 'USER' command )
```

## Development

```
# build && testing
$ vagrant up
```