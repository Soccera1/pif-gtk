# pif

A simple song management tool with both CLI and GTK interfaces.

## Building

### When you encounter a `code block`, run it in a terminal if required.

### Dependencies

- C compiler (GCC recommended)
- GTK+ 3.0 development files (for GTK interface)
- pkg-config
- libnotify-bin (for desktop notifications)
- polkitd (for privilege escalation via pkexec)

On Debian/Ubuntu, you can install the dependencies with:
```bash
sudo apt-get install build-essential libgtk-3-dev pkg-config libnotify-bin polkitd
```

### Compilation

```bash
make
```

This will build both the CLI version (`pif`) and the GTK version (`pif-gtk`).
**You MUST keep pif as pif-gtk depends on pif.**

## Installation

```bash
sudo make install
```

This will install both versions to `/usr/local/bin/` by default.

## Usage

### CLI Version

Run `pif` to start the command-line interface.

### GTK Version

Run `pif-gtk` from an application launcher to start the graphical interface.

## Features

- Add songs to your list
- Remove songs from your list
- View all songs
- Simple and intuitive interface (GTK version)
- Command-line interface for automation (CLI version)

## Visual demonstration
![2025-06-29-131018_1920x1200_scrot](https://github.com/user-attachments/assets/bc2bc5dd-75f1-4868-9f3d-675407968827)


## License

This project is licensed under the GNU Affero General Public License v3.0 - see the [LICENSE](LICENSE) file for details.
