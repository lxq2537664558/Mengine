set( MY_LIB_NAME SDLThreadSystem )

ADD_FILTER(
src
	SDLThreadSystem.cpp
	SDLThreadSystem.h
	SDLThreadIdentity.cpp
    SDLThreadIdentity.h
    SDLThreadMutex.h
    SDLThreadMutex.cpp
    SDLThreadConditionVariable.h
    SDLThreadConditionVariable.cpp
)

INCLUDE_DIRECTORIES( ${THIRDPARTY_DIR}/SDL2/include )

ADD_MENGINE_LIBRARY()

set_target_properties (${MY_LIB_NAME} PROPERTIES
    FOLDER Systems
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
 PARENT_SCOPE
)
