# Title

`ProcessMemoryScanner` is a Windows command-line utility to scan specified process for specified value. You can scan a process memory using user-mode or kernel-mode routines. To use kernel-mode routine you need `WdmMemoryReadWriteDriver` loaded.

## Install

```
```

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

## License
