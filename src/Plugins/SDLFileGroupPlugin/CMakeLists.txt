set( MY_LIB_NAME SDLFileGroupPlugin )

ADD_FILTER(
src
    SDLFileGroupPlugin.h
    SDLFileGroupPlugin.cpp
    SDLFileGroupDirectory.cpp
    SDLFileGroupDirectory.h
	SDLFileGroupDirectoryFactory.h
	SDLFileGroupDirectoryFactory.cpp
    SDLFileInputStream.h
    SDLFileInputStream.cpp
    SDLFileOutputStream.cpp
    SDLFileOutputStream.h
    SDLFileMappedStream.h
    SDLFileMappedStream.cpp
	SDLFileHelper.h
	SDLFileHelper.cpp
)

INCLUDE_DIRECTORIES(
    ${THIRDPARTY_DIR}/SDL2/include
    ${THIRDPARTY_DIR}/libmath/include
)

ADD_MENGINE_LIBRARY()

set_target_properties (${MY_LIB_NAME} PROPERTIES
    FOLDER Plugins
)

SET( APPLICATION_DEPENDENCIES
    ${APPLICATION_DEPENDENCIES}
    ${MY_LIB_NAME}
)

SET( APPLICATION_DEPENDENCIES
    ${APPLICATION_DEPENDENCIES}
    PARENT_SCOPE
)

SET( APPLICATION_LIBS
    ${APPLICATION_LIBS}
    Kernel
)

SET( APPLICATION_LIB_PATHS
    ${APPLICATION_LIB_PATHS}
    PARENT_SCOPE
)

SET( APPLICATION_LIBS
    ${APPLICATION_LIBS}
    PARENT_SCOPE
)
