#ifndef MACROSH_
#define MACROSH_

// More like constants but I'm not making yet another header file just for this
#ifndef LIDM_CONF_PATH
  #define LIDM_CONF_PATH "/etc/lidm.ini"
#endif

// Do we just replace the compiler with clang??
#ifdef __clang__
  #define NULLABLE _Nullable
#else
  #define NULLABLE
#endif

#ifdef __clang__
  #define NNULLABLE _Nonnull
#else
  #define NNULLABLE
#endif

#ifdef __clang__
  #define UNULLABLE _Null_unspecified
#else
  #define UNULLABLE
#endif

#ifdef __clang__
  #define COMPILER_VERSION __VERSION__
#elif defined(__GNUC__)
  #define xstr(s) str(s)
  #define str(s) #s

  #define COMPILER_VERSION \
    "GCC " xstr(__GNUC__) "." xstr(__GNUC_MINOR__) "." xstr(__GNUC_PATCHLEVEL__)
#endif

#define LEN(X) (sizeof(X) / sizeof((X)[0]))
#define UNUSED(x) ((void)(x))

#endif
