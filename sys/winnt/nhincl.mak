# =========================================================
# NTWIN32.MAK
# Win32 Application NMAKE Definitions
# Modified slightly for compiling NT NetHack 3.1.0
# =========================================================

# ---------------------------------------------------------
# Get CPU Type - exit if CPU environment variable is not defined
# ---------------------------------------------------------

# declarations for use on Intel 80x86 systems.
!IF "$(CPU)" == "i386"
CPUTYPE = 1
DLLENTRY = @12
!ENDIF

# declarations for use on self hosted MIPS systems.
!IF "$(CPU)" == "MIPS"
CPUTYPE = 2
DLLENTRY = 
!ENDIF

!IFNDEF CPUTYPE
!ERROR  Must specify CPU Environment Variable ( CPU=i386 or CPU=MIPS )
!ENDIF

# ---------------------------------------------------------
# Target Module Dependant Compile Declarations
#
# Below is a table which describes which flags to use
# depending on the module target:
#
# Module       Number of Threads Variables to C run-time
# Target       Single/Multiple   Include      Library
# -----------  ----------------- ------------ ----------
# MODULE .EXE  Single            CVARS        LIBC.LIB
# MODULE .EXE  Multiple          CVARSMT      LIBCMT.LIB
# MODULE .DLL  Single            CVARSDLL     CRTDLL.LIB
# MODULE .DLL  Multiple          CVARSMTDLL   CRTDLL.LIB
#
# Legend:
# MODULE  : A Win32 Graphical User Interface module or a
#           Win32 Character-Mode User Interface module
# ---------------------------------------------------------

cvars      = -DWIN32
cvarsmt    = $(cvars) -D_MT
cvarsdll   = $(cvars) -D_DLL
cvarsmtdll = $(cvars) -D_MT -D_DLL

#----------------------------------------------------------
# Subsystem Dependent Compile Declarations
#
# When compiling for the POSIX Subsystem, psxvars should be
# included.
#
#----------------------------------------------------------

psxvars    = -D_POSIX_

# ---------------------------------------------------------
# Platform Dependent Compile Flags - must be specified after $(cc)
#
# Note: Debug switches are default for current release
#
# These switches allow for source level debugging
# with WinDebug for local and global variables.
#
# i386 flags:
#   -c   - compile without linking
#   -G3  - generate 80386 instructions
#   -W3  - Set warning level to level 3
#   -Zi  - generate debugging information
#   -Od  - disable all optimizations
#
# MIPS flags:
#   -c   - compile without linking
#   -std - produce warnings for non-ANSI standard source code.
#   -g2  - produce a symbol table for debugging
#   -O   - invoke the global optimizer
#   -EL  - produce object modules targeted for
#          "little-endian" byte ordering
# ---------------------------------------------------------

# declarations for use on Intel 80x86 systems.
!IF "$(CPU)" == "i386"
cdebug   = -Zi -Od
cflags   = -c -G3 -W0 -Di386=1 -nologo
cvtdebug =
!ENDIF

# declarations for use on self hosted MIPS systems.
!IF "$(CPU)" == "MIPS"
cdebug   = -g2
cflags   = -c -std -o $(*B).obj -EL -DMIPS=1
cvtdebug = -c
!ENDIF

# ---------------------------------------------------------
# Target Module Dependent Link Flags - must be specified after $(link)
#
# Note: Debug switches are default for current release
#
# These switches allow for source level debugging
# with WinDebug for local and global variables.
# ---------------------------------------------------------

linkdebug = -debug:full -debugtype:cv
conflags  = -subsystem:console -entry:mainCRTStartup
guiflags  = -subsystem:windows -entry:WinMainCRTStartup
psxflags  = -subsystem:posix -entry:__PosixProcessStartup

# ---------------------------------------------------------
# Platform Dependent Binaries Declarations
#
# Note: Debug switches are default for current release
#
# These switches allow for source level debugging
# with WinDebug for local and global variables.
# ---------------------------------------------------------

# declarations for use on Intel 80x86 systems.
!IF "$(CPU)" == "i386"
cc     = cl386
cvtobj = REM MIPS-only conversion:
!ENDIF

# declarations for use on self hosted MIPS systems.
!IF "$(CPU)" == "MIPS"
cc     = cc
cvtobj = mip2coff 
!ENDIF

link = link 

# ---------------------------------------------------------
# Target Module Dependent Link Libraries
#
# Below is a table which describes which libraries to use
# depending on the module target:
#
# Module       Number of Threads Variables to C run-time
# Target       Single/Multiple   Include      Library
# -----------  ----------------- ------------ ----------
# CONSOLE.EXE  Single            CONLIBS      LIBC.LIB
# CONSOLE.EXE  Multiple          CONLIBSMT    LIBCMT.LIB
# CONSOLE.DLL  Either            CONLIBSDLL   CRTDLL.LIB
# WINDOWS.EXE  Single            GUILIBS      LIBC.LIB
# WINDOWS.EXE  Multiple          GUILIBSMT    LIBCMT.LIB
# WINDOWS.DLL  Either            GUILIBSDLL   CRTDLL.LIB
# POSIX.EXE    Single            PSXLIBS      LIBCPSX.LIB
#
# Legend:
# WINDOWS : A Win32 Graphical User Interface module
# CONSOLE : A Win32 Character-Mode User Interface module
# POSIX   : A Posix Subsystem Character-Mode User Interface module
# ---------------------------------------------------------

conlibs    = libc.lib ntdll.lib kernel32.lib

conlibsmt  = libcmt.lib ntdll.lib kernel32.lib

conlibsdll = crtdll.lib ntdll.lib kernel32.lib

guilibs    = libc.lib ntdll.lib kernel32.lib user32.lib gdi32.lib \
             winspool.lib comdlg32.lib

guilibsmt  = libcmt.lib ntdll.lib kernel32.lib user32.lib gdi32.lib \
             winspool.lib comdlg32.lib

guilibsdll = crtdll.lib ntdll.lib kernel32.lib user32.lib gdi32.lib \
             winspool.lib comdlg32.lib

psxlibs    = libcpsx.lib ntdll.lib kernel32.lib psxdll.lib psxrtl.lib
