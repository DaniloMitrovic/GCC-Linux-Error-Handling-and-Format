# GCC-Linux-Error-Handling-and-Format
A layered error handling without use of "iostream", purely reliant on "ctime" (and "cstdio") and gcc builtins. Enables various layers of "panics" and calls.
Its intended use is to be used when developing command line tools that should work with bash, simplyfing concepts and allowing for splitting up complex projects into smaller parts. File `sandbox.cpp` was used for testing.

Disclaimer #1: This is tested and working on Ubuntu Numbat. I haven't implemented any NON-POSIX variations, feel free to contribute.  

Disclaimer #2: It requires GCC >= 4.7. Its provided with Ubuntu (and some other distros) so CLANG and MSVS aren't implemented, feel free to contribute.

Disclaimer #3: "Divide et impera" - error byte size can be expanded for more complex projects. But my personal belief is that such projects benefit a lot from separation. I will most likely choose not to expand error code size.

Disclaimer #4: "Can't have your cake and eat it too" - On every PANIC error code will be greater or equal than 1, and by assigning different codes you can expand that up to core 127 POSIX shell exit codes. The rest of error code is descriptive only - ie printed and returned with `end_code`. 

## Layers 

There are 3 layers of error codes:
  1. What is printed : ie expanded error code containing exit code (1)
  2. What can be debugged: ie error code that is provided via 'end_code'
  3. What is exited : first nibble of exit code is reserved for program exit (in panic) so that it can be picked up by bash.

## Error code construction:

Error codes are meant not as random integer values but as standardisation of developer intent. They are described by unsigned int (minimum 4 bytes) that has next structure:

| B3 | B2 | B1 | B0 | description  |
|---|---|---|---|---|
| 00 | 00 | 00 | XX | status and global exit code (1) |
| 00 | 00 | XX | 00 | module/object exit code (whatever)  |
| 00 | XX | 00 | 00 | system/orchestrator exit code (whatever)  |
| XX | 00 | 00 | 00 | main exit code ( as in `int main` main)  |

So one can layer and continue error checking various inheritances, dependancies and so on by signaling specific error code fields.

## Error code recomendation

Id suggest structuring error codes as bit flags. Any `dm_gcc_probe` object will return `end_code` (ie what was inputed) and will not return exit code (that is left for after program finishes). Thus its recomended to keep any code handling within nibble structure (providing 4 independant flags) per error code nibble. Thus every error code byte can be considered as 0xLG (L: local, G: global).
  - Global bits (0x0f) are mostly used as exit codes or can be repurposed as such. so creating, initialisating, destruction and assingment - create, init, destroy and assign.
  - Local bits (0xf0) are mostly used as status codes or can be repurposed as such, so getting, setting, clearing and checking - get,set,clear,has.

By using this method, it should be easy to flip codes that are handled or annote them as such like (set + get = assign) or something similar. Combining that with messaging , it should be easy to navigate to correct issue/problem at hand. 

## Error code reservation
Only first nibble is reserved for system errors. It will allways exit with 1 (if panic), so only thing that one should keep in mind is how to check those exit codes when in shell. For regular shells/builtins codes are as they are, for this one, know if program should return value with 1, or if that 1 should be ignored.

## Probe Panics
 - Panic: (when panic is set to true) will terminate the program and show exit code. Should be used in release build or to stop program mid development to inspect a quick bug.
 - Non Panic: (when panic is set to false) will not terminate the program but only print error in stderr. Shouldn't be used in release build (to ensure clarity) and it allows developer to handle priorities when developing.

Basically: "Keep calm and take it one bug at the time" mentality. 
