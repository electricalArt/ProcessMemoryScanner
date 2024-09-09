# Title

`ProcessMemoryScanner` is a Windows command-line utility to scan specified process for specified value. You can scan a process memory using user-mode or kernel-mode routines. To use kernel-mode routine you need `WdmMemoryReadWriteDriver` installed.

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
                  --process-id <process_id>
                  --value-type   could be:
                      int32
                      int64 
                      float 
                      double
                  --driver-mode
                  --version
                  -h, --help
```

## Contributing

PRs are accepted.

## License
