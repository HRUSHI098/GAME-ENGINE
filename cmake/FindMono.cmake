# FindMono.cmake
# Locates the Mono embedding SDK (libmono-sgen + headers).
#
# Defines:
#   MONO_FOUND          — TRUE if found
#   MONO_INCLUDE_DIRS   — path to mono/jit/jit.h etc.
#   MONO_LIBRARIES      — mono-sgen / monosgen-2.0 lib
#   MONO_EXECUTABLE     — path to the mono / mcs executables
#   Mono::Mono          — imported target
#
# Hints (set before find_package):
#   MONO_ROOT           — override root install dir

# ── pkg-config path (Linux / macOS) ──────────────────────────────────────────
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(PC_MONO QUIET mono-2)
endif()

# ── Windows default install locations ────────────────────────────────────────
set(_MONO_WIN_ROOTS
    "C:/Program Files/Mono"
    "C:/Program Files (x86)/Mono"
)

# ── Headers ───────────────────────────────────────────────────────────────────
find_path(MONO_INCLUDE_DIR
    NAMES mono/jit/jit.h
    HINTS
        ${MONO_ROOT}
        ${PC_MONO_INCLUDEDIR}
        ${PC_MONO_INCLUDE_DIRS}
    PATH_SUFFIXES
        include/mono-2.0
        mono-2.0
    PATHS ${_MONO_WIN_ROOTS}
)

# ── Library ───────────────────────────────────────────────────────────────────
find_library(MONO_LIBRARY
    NAMES mono-2.0-sgen mono-2.0 monosgen-2.0 mono-sgen libmonosgen-2.0
    HINTS
        ${MONO_ROOT}
        ${PC_MONO_LIBDIR}
        ${PC_MONO_LIBRARY_DIRS}
    PATH_SUFFIXES lib lib64
    PATHS ${_MONO_WIN_ROOTS}
)

# ── Executables ───────────────────────────────────────────────────────────────
find_program(MONO_EXECUTABLE   NAMES mono   HINTS ${MONO_ROOT}/bin PATHS ${_MONO_WIN_ROOTS})
find_program(MCS_EXECUTABLE    NAMES mcs    HINTS ${MONO_ROOT}/bin PATHS ${_MONO_WIN_ROOTS})
find_program(DOTNET_EXECUTABLE NAMES dotnet)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Mono
    REQUIRED_VARS MONO_LIBRARY MONO_INCLUDE_DIR
)

if(MONO_FOUND AND NOT TARGET Mono::Mono)
    add_library(Mono::Mono UNKNOWN IMPORTED)
    set_target_properties(Mono::Mono PROPERTIES
        IMPORTED_LOCATION             "${MONO_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${MONO_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(MONO_INCLUDE_DIR MONO_LIBRARY MONO_EXECUTABLE MCS_EXECUTABLE)
