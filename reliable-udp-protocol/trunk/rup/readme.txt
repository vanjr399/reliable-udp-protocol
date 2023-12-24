RUP - Reliable UDP

Author: John Van Drasek
Date: 5/11/2008


To compile the windows version of RUP, uncomment the line in rup.h "#define _WIN32_".  Rebuild the project and the lib will end up in the bin directory for either rup32d.lib or rup32r.lib depending if you are compiling in release or debug mode.

TODO:
- Use warning level 4 and keep warnings as errors.  To do this we need to get rid of the warnings produced by the FD_SET function.