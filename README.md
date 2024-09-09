# Title

`ProcessMemoryScanner` is a Windows command-line utility to scan specified process for specified value. It searches for the process virtual memory addresses that point to specified value. Utlity prints found addresses to standard output and writes them to `addresses.txt`.  <br/>
<br/>
You can scan a process memory using user-mode or kernel-mode routines. To use kernel-mode routine you need `WdmMemoryReadWriteDriver` loaded.

## Install

To receive ready-to-use executable, take a look to [releases](https://github.com/elektrikArt/ProcessMemoryScanner/releases) page.

### Build

To build the executable by yourself, you should previously have [`MemoryHackerLib`](https://github.com/elektrikArt/MemoryHackerLib), [`SharedStuffLib`](https://github.com/elektrikArt/SharedStuffLib), [`easylogging++`](https://github.com/abumq/easyloggingpp) and [`tclap`](https://github.com/mirror/tclap) projects installed and linked. Then, use standard Visual Studio build routine.

## Usage

```
ProcessMemoryScanner [<command>] <value> [<options>]

The following commands are available:
    search
    filter
    write

The following options are available:
     --driver-mode
       Switch to use kernel driver to access specified process
  
     --value-type <int32 | int64 | float | double>
       (required)  The following types are avaiable: int32 | int64 | float |
       double
  
     --process-id <id>
       (required)
  
     --,  --ignore_rest
       Ignores the rest of the labeled arguments following this flag.
  
     --version
       Displays version information and exits.
  
     -h,  --help
       Displays usage information and exits.
```

## Contributing

PRs are accepted.
