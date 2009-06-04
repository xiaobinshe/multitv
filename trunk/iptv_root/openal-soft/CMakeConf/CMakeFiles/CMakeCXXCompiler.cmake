SET(CMAKE_CXX_COMPILER "/usr/bin/c++")
SET(CMAKE_CXX_COMPILER_ARG1 "")
SET(CMAKE_AR "/usr/bin/ar")
SET(CMAKE_RANLIB "/usr/bin/ranlib")
SET(CMAKE_COMPILER_IS_GNUCXX 1)
SET(CMAKE_CXX_COMPILER_LOADED 1)
SET(CMAKE_COMPILER_IS_MINGW )
SET(CMAKE_COMPILER_IS_CYGWIN )
IF(CMAKE_COMPILER_IS_CYGWIN)
  SET(CYGWIN 1)
  SET(UNIX 1)
ENDIF(CMAKE_COMPILER_IS_CYGWIN)

SET(CMAKE_CXX_COMPILER_ENV_VAR "CXX")

IF(CMAKE_COMPILER_IS_MINGW)
  SET(MINGW 1)
ENDIF(CMAKE_COMPILER_IS_MINGW)
SET(CMAKE_COMPILER_IS_GNUCXX_RUN 1)
SET(CMAKE_CXX_IGNORE_EXTENSIONS inl;h;H;o;O;obj;OBJ;def;DEF;rc;RC)
SET(CMAKE_CXX_SOURCE_FILE_EXTENSIONS C;M;c++;cc;cpp;cxx;m;mm)
SET(CMAKE_CXX_LINKER_PREFERENCE Prefered)
IF(UNIX)
  SET(CMAKE_CXX_OUTPUT_EXTENSION .o)
ELSE(UNIX)
  SET(CMAKE_CXX_OUTPUT_EXTENSION .obj)
ENDIF(UNIX)
# save the size of void* in case where cache is removed
# and the this file is still around
SET(CMAKE_SIZEOF_VOID_P )
