set( MY_LIB_NAME MoviePlugin )

ADD_FILTER(
src
    Movie2Interface.h
    
	MoviePlugin.h
	MoviePlugin.cpp
    
    Movie2.cpp
    Movie2.h
    Movie2Slot.cpp
    Movie2Slot.h
    Movie2Scissor.cpp
    Movie2Scissor.h
    Movie2SubComposition.cpp
    Movie2SubComposition.h
    ResourceMovie2.cpp
    ResourceMovie2.h
    LoaderResourceMovie2.cpp
    LoaderResourceMovie2.h
    
    Movie2DebugRenderVisitor.cpp
    Movie2DebugRenderVisitor.h
    Movie2ResourceValidateVisitor.cpp
    Movie2ResourceValidateVisitor.h
)

INCLUDE_DIRECTORIES( ${THIRDPARTY_DIR}/stdex/include )
INCLUDE_DIRECTORIES( ${THIRDPARTY_DIR}/libmovie/include )
INCLUDE_DIRECTORIES( ${THIRDPARTY_DIR}/pybind/include )
INCLUDE_DIRECTORIES( ${THIRDPARTY_DIR}/libmath/include )
INCLUDE_DIRECTORIES( ${THIRDPARTY_DIR}/metabuf )

ADD_MENGINE_LIBRARY()

set_target_properties (${MY_LIB_NAME} PROPERTIES
    FOLDER Plugins
)

SET( APPLICATION_DEPENDENCIES
 ${APPLICATION_DEPENDENCIES}
 ${MY_LIB_NAME}
 Kernel
 Engine
)

SET( APPLICATION_DEPENDENCIES
 ${APPLICATION_DEPENDENCIES}
 PARENT_SCOPE
)

SET( APPLICATION_LIBS
    ${APPLICATION_LIBS}
	${THIRDPARTY_LIB_DIR}/${MENGINE_LIB_PREFIX}stdex${MENGINE_LIB_SUFFIX}
	${THIRDPARTY_LIB_DIR}/${MENGINE_LIB_PREFIX}pybind${MENGINE_LIB_SUFFIX}
	${THIRDPARTY_LIB_DIR}/${MENGINE_LIB_PREFIX}math${MENGINE_LIB_SUFFIX}	
    ${THIRDPARTY_LIB_DIR}/${MENGINE_LIB_PREFIX}metabuf${MENGINE_LIB_SUFFIX}
	${THIRDPARTY_LIB_DIR}/${MENGINE_LIB_PREFIX}movie${MENGINE_LIB_SUFFIX}	
    )

SET( APPLICATION_LIBS
 ${APPLICATION_LIBS}
 PARENT_SCOPE
)