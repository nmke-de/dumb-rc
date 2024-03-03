# Dumb RunCommand

**Dumb RunCommand** (short: drc) is a program that scans directories for services to supervise. By default, the supervisor will be [`/usr/local/bin/dsv`](https://github.com/nmke-de/dumb-sv).

## But why?

The same reason why other init systems have `runsvdir` (for runit), `svscan` (for daemontools-encore) and so on. It simplifies the process of starting supervised services.

## (Build) Dependencies

- [Dietlibc](https://www.fefe.de/dietlibc/) - other libc are not supported.
- [TCC](https://bellard.org/tcc/) - GCC is not supported.
- [GNU Binutils](https://www.gnu.org/software/binutils/)
- [Make](https://www.gnu.org/software/make/)

## Build

Type `make`.

## Usage

```bash
drc [directories...]
```

The directories are expected to contain executable regular files that run indefinitely.

The location of dsv can be given by setting the environment variable `DSV_LOCATION`.
