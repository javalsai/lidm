// Do we just replace the compiler with clang??
#if defined(__clang__)
  #define NULLABLE _Nullable
#else
  #define NULLABLE
#endif

#if defined(__clang__)
  #define NNULLABLE _Nonnull
#else
  #define NNULLABLE
#endif
