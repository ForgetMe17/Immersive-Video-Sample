CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

OPTION(LINUX_OS
       "Use linux os"
       ON
)

OPTION(ANDROID_OS
       "Use android os"
       OFF
)

PROJECT(app)

IF(LINUX_OS)
  ADD_DEFINITIONS("-g -c -fPIC -lglog -std=c++11 -fpermissive")
ENDIF()

IF(LINUX_OS)
  ADD_DEFINITIONS("-D_LINUX_OS_")
ENDIF()

IF(ANDROID_OS)
  ADD_DEFINITIONS("-D_ANDROID_OS_")
ENDIF()

IF(LINUX_OS)
  INCLUDE_DIRECTORIES(/usr/local/include ../../utils)
  LINK_DIRECTORIES(/usr/local/lib)
ENDIF()

AUX_SOURCE_DIRECTORY(. DIR_SRC)
AUX_SOURCE_DIRECTORY(android ANDROID_SRC)
AUX_SOURCE_DIRECTORY(linux LINUX_SRC)
IF(LINUX_OS)
  SET(DIR_SRC
      ${DIR_SRC}
      ${LINUX_SRC}
      )
  IF(DEFINED WEBRTC_LINUX_SDK)
      INCLUDE_DIRECTORIES("${WEBRTC_LINUX_SDK}/include")
      LINK_DIRECTORIES("${WEBRTC_LINUX_SDK}/lib")
  ENDIF()
ENDIF()

IF(ANDROID_OS)
  SET(DIR_SRC
      ${DIR_SRC}
      ${ANDROID_SRC}
      )
ENDIF()

ADD_EXECUTABLE(render ${DIR_SRC})

IF(LINUX_OS)
  SET(LINK_LIB ${LINK_LIB} MediaPlayer glog glfw GL GLU safestring_shared)
ENDIF()

TARGET_LINK_LIBRARIES(render ${LINK_LIB})
