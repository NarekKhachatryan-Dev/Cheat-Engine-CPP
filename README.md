# Linux Process Memory Scanner

Small educational C++ project that demonstrates how a Linux process exposes its
memory through the `/proc` filesystem.

The project contains two programs:

- `target` keeps an integer in memory and waits for user input.
- `scanner` searches the target process for that integer, narrows the results,
  and writes a replacement value to the first remaining address.

> This is a learning project for local experiments with processes you own.
> Do not use it to inspect or modify software without authorization.

## How It Works

The scanner follows a simple multi-step workflow:

1. Reads `/proc/<PID>/maps` and collects private read/write regions (`rw-p`).
2. Reads those regions through `/proc/<PID>/mem`.
3. Searches for byte sequences that match a 32-bit `int` value.
4. Re-reads each candidate during filtering and keeps matching addresses.
5. Writes the replacement integer to the first remaining candidate.

This mirrors the basic idea behind a value scanner while keeping the code small
enough to study in one sitting.

## Requirements

- Linux
- A C++ compiler with C++20 support
- Permission to read and write the selected process memory

The code uses Linux-specific interfaces, including `/proc`, `getpid()`, and
process memory permissions, so it is not portable to Windows or macOS as-is.

## Build

From the project directory:

```bash
g++ -std=c++20 -Wall -Wextra -pedantic target.cpp -o target
g++ -std=c++20 -Wall -Wextra -pedantic scanner.cpp -o scanner
```

## Run the Demonstration

Open two terminals in this directory.

### Terminal 1: start the target

```bash
./target
```

The program prints its PID, the address of the integer, and its initial value:

```text
Target PID: 1234
Target Address: 0x7ffd...
Target value: 12345
```

Leave it waiting for input and copy the printed PID.

### Terminal 2: scan and change the value

```bash
sudo ./scanner
```

Then enter:

1. The PID printed by `target`.
2. `12345` as the first value to search for.
3. A new value observed in the target when filtering, or `-1` to skip
   filtering and continue.
4. The replacement value to write.

For example, the final value can be `99999`. Return to the first terminal and
enter `1`; the target should print the modified value before exiting.

## Permissions

Linux may deny access to `/proc/<PID>/mem` because of process isolation,
ownership, or the system's Yama `ptrace_scope` setting. If access fails, use a
debugging session with processes owned by the same user and check the relevant
permissions on your system. Avoid weakening system security settings unless you
understand the consequences.

## Project Structure

```text
.
├── README.md       # Project documentation
├── target.cpp      # Test process with a known integer value
└── scanner.cpp     # Region scanner, candidate filter, and memory writer
```

## Current Limitations

- Scans only regions whose permissions are exactly `rw-p`.
- Searches only values represented by a C++ `int`.
- Loads each complete writable region into memory before scanning it.
- Does not verify that a candidate remains writable immediately before writing.
- Changes only the first remaining candidate address.
- Assumes the target process remains alive while scanning.
- Does not provide a portable abstraction for other operating systems.

These constraints are intentional for a compact demonstration, but they would
need to be addressed in a production-quality diagnostic tool.

## Learning Topics

This example is useful for studying:

- Linux process memory maps
- File streams and random access with `seekg` and `seekp`
- Raw byte buffers and integer representation
- Candidate filtering across repeated observations
- Process permissions and the security model around `ptrace`

## License

No license has been specified for this repository yet. Add a license before
redistributing the code or accepting contributions under defined terms.

## Disclaimer

This project is strictly for educational purposes. Do not use it on multiplayer games or software you do not own.