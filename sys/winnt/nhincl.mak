# =========================================================================
# NHINCL.MAK - Modified Win32 application master NMAKE definitions file for
#              NetHack 3.1.3 - NetHack PC Development Team 93/07/12
# -------------------------------------------------------------------------
# NMAKE Options
#
# Use the table below to determine the extra options for NMAKE to
# generate various application debugging, profiling and performance tuning
# information.
#
# Application Information Type         Invoke NMAKE
# ----------------------------         ------------
# For No Debugging Info                nmake nodebug=1
# For Working Set Tuner Info           nmake tune=1
# For Call Attributed Profiling Info   nmake profile=1
#
# Note: Working Set Tuner and Call Attributed Profiling is for Intel
#       x86 systems only.
#
# Note: The three options above are mutually exclusive (you may use only
#       one at complile/link the application).
#
# Note: using the environment variables NODEBUG, TUNE, and PROFILE is an
#       alternate method for setting these options.
#
# Additional NMAKE Options             Invoke NMAKE
# ----------------------------         ------------
# For No ANSI NULL Compliance          nmake no_nansi=1
# (ANSI NULL is defined as PVOID 0)
#
# =========================================================================

# -------------------------------------------------------------------------
# Get CPU Type - exit if CPU environment variable is not defined
# -------------------------------------------------------------------------

# Intel i386 and i486 systems.
!IF "$(CPU)" == "i386"
CPUTYPE = 1
!ENDIF

# MIPS R4000 systems.
!IF "$(CPU)" == "MIPS"
CPUTYPE = 2
!ENDIF

!IFNDEF CPUTYPE
!ERROR  Must specify CPU environment variable ( CPU=i386 or CPU=MIPS )
!ENDIF

# -------------------------------------------------------------------------
# Platform Dependent Binaries Declarations
#
# If you are using the old Mips compiler then
# cc     = cc
# cvtobj = mip2coff
# -------------------------------------------------------------------------

# binary declarations for use on Intel i386 and i486 systems.
!IF "$(CPU)" == "i386"
cc     = cl386
cvtobj = REM !!! CVTOBJ is no longer necessary - please remove !!!
!ENDIF

# binary declarations for use on self hosted MIPS R4000 systems.
!IF "$(CPU)" == "MIPS"
cc     = mcl
cvtobj = REM !!! CVTOBJ is no longer necessary - please remove !!!
!ENDIF

# binary declarations common to all platforms.
link   = link32
implib = lib32
rc     = rc
cvtres = cvtres
hc     = hc

# -------------------------------------------------------------------------
# Platform Dependent Compile Flags - must be specified after $(cc)
#
# Note: Debug switches are on by default for current release
#
# These switches allow for source level debugging with WinDebug for local
# and global variables.
#
# Both compilers now use the same front end - you must still define either
# _X86_ or _MIPS_.  These have replaced the i386 and MIPS definitions which
# are not ANSI compliant.
#
# i386 specific compiler flags:
#   -G3  - generate 80386 instructions
#   -Gz  - stdcall
#
# MS MIPS specific compiler flags:
#
#
# Common compiler flags:
#   -c   - compile without linking
#   -W3  - Set warning level to level 3
#   -Zi  - generate debugging information
#   -Od  - disable all optimizations
#   -Ox  - use maximum optimizations
#   -Zd  - generate only public symbols and line numbers for debugging
#
# *** OLD MIPS ONLY ***
#
# The following definitions are for the old Mips compiler.
#
# OLD MIPS compiler flags:
#   -c   - compile without linking
#   -std - produce warnings for non-ANSI standard source code.
#   -g2  - produce a symbol table for debugging
#   -O   - invoke the global optimizer
#   -EL  - produce object modules targeted for
#          "little-endian" byte ordering
# -------------------------------------------------------------------------

# declarations common to all compilers.
ccommon = -c -W0 -nologo

!IF "$(CPU)" == "i386"
cflags = $(ccommon) -G3 -D_X86_=1
scall  = -Gz
!ELSE
cflags = $(ccommon) -D_MIPS_=1
scall  =
!ENDIF

!IF "$(CPU)" == "i386"
!IFDEF NODEBUG
cdebug = -Ox
!ELSE
!IFDEF PROFILE
cdebug = -Gh -Zd -Ox
!ELSE
!IFDEF TUNE
cdebug = -Gh -Zd -Ox
!ELSE
cdebug = -Zi -Od
!ENDIF
!ENDIF
!ENDIF
!ELSE
!IFDEF NODEBUG
cdebug = -Ox
!ELSE
cdebug = -Zi -Od
!ENDIF
!ENDIF

# OLD MIPS Complile Flags
!IF 0
!IF "$(CPU)" == "MIPS"
cflags = -c -std -o $(*B).obj -EL -DMIPS=1 -D_MIPS_=1
!IFDEF NODEBUG
cdebug =
!ELSE
cdebug = -g2
!ENDIF
!ENDIF
!ENDIF

# -------------------------------------------------------------------------
# Target Module & Subsystem Dependant Compile Defined Variables - must be
#   specified after $(cc)
#
# The following is a table which indicates the various
# acceptable combinations of CRTDLL and LIBC(MT) with
# respect to DLLs and EXEs, along with the appropriate
# compiler flags for each.
#
# Link EXE    Create Exe    Link DLL    Create DLL
#   with        Using         with         Using
# ----------------------------------------------------
#  LIBC        CVARS          None        None      *
#  LIBC        CVARS          LIBC        CVARS
#  LIBC        CVARS          LIBCMT      CVARSMT
#  LIBCMT      CVARSMT        None        None      *
#  LIBCMT      CVARSMT        LIBC        CVARS
#  LIBCMT      CVARSMT        LIBCMT      CVARSMT
#  CRTDLL      CVARSDLL       None        None      *
#  CRTDLL      CVARSDLL       LIBC        CVARS
#  CRTDLL      CVARSDLL       LIBCMT      CVARSMT
#  CRTDLL      CVARSDLL       CRTDLL      CVARSDLL  *
#
# Note: Any executable which accesses a DLL linked with CRTDLL.LIB must
#       also link with CRTDLL.LIB instead of LIBC.LIB or LIBCMT.LIB.
#       When using DLLs, it is recommended that all of the modules be
#       linked with CRTDLL.LIB.
#
# * - Recommended Configurations
#
# -------------------------------------------------------------------------

!IFDEF NO_ANSI
noansi = -DNULL=0
!ENDIF
cvars      = -DWIN32 $(noansi)
cvarsmt    = $(cvars) -D_MT
cvarsdll   = $(cvarsmt) -D_DLL
cvarsmtdll = $(cvarsmt) -D_DLL

# For POSIX applications
psxvars    = -D_POSIX_

# resource compiler
rcvars = -DWIN32 $(noansi)

# -------------------------------------------------------------------------
# Platform Dependent Link Flags - must be specified after $(link)
#
# Note: $(DLLENTRY) should be appended to each -entry: flag on the link line.
# -------------------------------------------------------------------------

# declarations for use on Intel i386 and i486 systems.
!IF "$(CPU)" == "i386"
DLLENTRY = @12
!ENDIF

# declarations for use on self hosted MIPS R4000 systems.
!IF "$(CPU)" == "MIPS"
DLLENTRY =
!ENDIF

# -------------------------------------------------------------------------
# Target Module Dependent Link Debug Flags - must be specified after $(link)
#
# These switches allow for source level debugging with WinDebug, profiling
# or performance tuning.
#
# Note: Debug switches are on by default.
# -------------------------------------------------------------------------

!IF "$(CPU)" == "i386"
!IFDEF NODEBUG
ldebug =
!ELSE
!IFDEF PROFILE
ldebug = -debug:partial -debugtype:coff
!ELSE
!IFDEF TUNE
ldebug = -debug:partial -debugtype:coff
!ELSE
ldebug = -debug:full -debugtype:cv
!ENDIF
!ENDIF
!ENDIF
!ELSE
!IFDEF NODEBUG
ldebug =
!ELSE
ldebug = -debug:full -debugtype:cv
!ENDIF
!ENDIF

# for compatibility with older style makefiles
linkdebug = $(ldebug)

# -------------------------------------------------------------------------
# Subsystem Dependent Link Flags - must be specified after $(link)
#
# These switches allow for source level debugging with WinDebug for local
# and global variables.
# -------------------------------------------------------------------------

# For applications that use the C-Runtime libraries
conlflags = -subsystem:console -entry:mainCRTStartup
guilflags = -subsystem:windows -entry:WinMainCRTStartup

# For POSIX applications
psxlflags = -subsystem:posix -entry:__PosixProcessStartup

# for compatibility with older style makefiles
conflags  = $(conlflags) -IGNORE:505
guiflags  = $(guilflags) -IGNORE:505
psxflags  = $(psxlflags)

# -------------------------------------------------------------------------
# C-Runtime Target Module Dependent Link Libraries
#
# Below is a table which describes which libraries to use depending on the
# target module type, although the table specifically refers to Graphical
# User Interface apps, the exact same dependancies apply to Console apps.
# I.E. you could replace all occurences of GUI with CON in the following:
#
# Desired CRT  Libraries   Desired CRT  Libraries
#   Library     to link      Library     to link
#   for EXE     with EXE     for DLL     with DLL
# ----------------------------------------------------
#   LIBC       GUILIBS       None       None       *
#   LIBC       GUILIBS       LIBC       GUILIBS
#   LIBC       GUILIBS       LIBCMT     GUILIBSMT
#   LIBCMT     GUILIBSMT     None       None       *
#   LIBCMT     GUILIBSMT     LIBC       GUILIBS
#   LIBCMT     GUILIBSMT     LIBCMT     GUILIBSMT
#   CRTDLL     GUILIBSDLL    None       None       *
#   CRTDLL     GUILIBSDLL    LIBC       GUILIBS
#   CRTDLL     GUILIBSDLL    LIBCMT     GUILIBSMT
#   CRTDLL     GUILIBSDLL    CRTDLL     GUILIBSDLL *
#
# * - Recommended Configurations.
#
# Note: Any executable which accesses a DLL linked with CRTDLL.LIB must
#       also link with CRTDLL.LIB instead of LIBC.LIB or LIBCMT.LIB.
#
# Note: For POSIX applications, link with $(psxlibs)
#
# -------------------------------------------------------------------------

# optional profiling and tuning libraries
!IF "$(CPU)" == "i386"
!IFDEF PROFILE
optlibs = cap.lib
!ELSE
!IFDEF TUNE
optlibs = wst.lib
!ELSE
optlibs =
!ENDIF
!ENDIF
!ELSE
optlibs =
!ENDIF

# basic subsystem specific libraries less the C-RunTime
baselibs   = kernel32.lib ntdll.lib $(optlibs) 
winlibs    = kernel32.lib ntdll.lib user32.lib gdi32.lib winspool.lib comdlg32.lib $(optlibs)

# For applications that use the C-Runtime libraries
conlibs    = libc.lib $(baselibs)
conlibsmt  = libcmt.lib $(baselibs)
conlibsdll = crtdll.lib $(baselibs)
guilibs    = libc.lib $(winlibs)
guilibsmt  = libcmt.lib $(winlibs)
guilibsdll = crtdll.lib $(winlibs)

# For POSIX applications
psxlibs    = libcpsx.lib psxdll.lib psxrtl.lib $(baselibs)
