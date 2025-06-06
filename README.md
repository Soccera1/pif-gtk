# pif

A simple song management tool with both CLI and GTK interfaces.

## Building

### Dependencies

- C compiler (GCC recommended)
- GTK+ 3.0 development files (for GTK interface)
- pkg-config

On Debian/Ubuntu, you can install the dependencies with:
```bash
sudo apt-get install build-essential libgtk-3-dev pkg-config
```

### Compilation

```bash
make
```

This will build both the CLI version (`pif`) and the GTK version (`pif-gtk`).

## Installation

```bash
sudo make install
```

This will install both versions to `/usr/local/bin/` by default.

## Usage

### CLI Version

Run `pif` to start the command-line interface.

### GTK Version

Run `pif-gtk` to start the graphical interface.

## Features

- Add songs to your list
- Remove songs from your list
- View all songs
- Simple and intuitive interface (GTK version)
- Command-line interface for automation (CLI version)

## License

This project is licensed under the GNU Affero General Public License v3.0 - see the [LICENSE](LICENSE) file for details.