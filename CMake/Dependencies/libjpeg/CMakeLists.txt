CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
                                             
project( jpeg )

SET( SOURCE_DIR ${THIRDPARTY_DIR}/libjpeg )

#configure_file( ${SOURCE_DIR}/jconfig.vc ${SOURCE_DIR}/jconfig.h COPYONLY)

IF(MENGINE_RENDER_DIRECTX)
    configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/jmorecfg_dx.h ${SOURCE_DIR}/jmorecfg.h COPYONLY )
ELSE()
    configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/jmorecfg_ogl.h ${SOURCE_DIR}/jmorecfg.h COPYONLY )
ENDIF()


configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/jconfig.vc ${SOURCE_DIR}/jconfig.h COPYONLY )

# OUR SOURCES
set( jpeg_sources
${SOURCE_DIR}/jaricom.c
${SOURCE_DIR}/jcapimin.c
${SOURCE_DIR}/jcapistd.c
${SOURCE_DIR}/jcarith.c
${SOURCE_DIR}/jccoefct.c
${SOURCE_DIR}/jccolor.c
${SOURCE_DIR}/jcdctmgr.c
${SOURCE_DIR}/jchuff.c
${SOURCE_DIR}/jcinit.c
${SOURCE_DIR}/jcmainct.c
${SOURCE_DIR}/jcmarker.c
${SOURCE_DIR}/jcmaster.c
${SOURCE_DIR}/jcomapi.c
${SOURCE_DIR}/jcparam.c
${SOURCE_DIR}/jcprepct.c
${SOURCE_DIR}/jcsample.c
${SOURCE_DIR}/jctrans.c
${SOURCE_DIR}/jdapimin.c
${SOURCE_DIR}/jdapistd.c
${SOURCE_DIR}/jdarith.c
${SOURCE_DIR}/jdatadst.c
${SOURCE_DIR}/jdatasrc.c
${SOURCE_DIR}/jdcoefct.c
${SOURCE_DIR}/jdcolor.c
${SOURCE_DIR}/jddctmgr.c
${SOURCE_DIR}/jdhuff.c
${SOURCE_DIR}/jdinput.c
${SOURCE_DIR}/jdmainct.c
${SOURCE_DIR}/jdmarker.c
${SOURCE_DIR}/jdmaster.c
${SOURCE_DIR}/jdmerge.c
${SOURCE_DIR}/jdpostct.c
${SOURCE_DIR}/jdsample.c
${SOURCE_DIR}/jdtrans.c
${SOURCE_DIR}/jerror.c
${SOURCE_DIR}/jfdctflt.c
${SOURCE_DIR}/jfdctfst.c
${SOURCE_DIR}/jfdctint.c
${SOURCE_DIR}/jidctflt.c
${SOURCE_DIR}/jidctfst.c
${SOURCE_DIR}/jidctint.c
${SOURCE_DIR}/jquant1.c
${SOURCE_DIR}/jquant2.c
${SOURCE_DIR}/jutils.c
${SOURCE_DIR}/jmemmgr.c
)

#choose memory manager
set(jpeg_sources ${jpeg_sources} ${CMAKE_CURRENT_SOURCE_DIR}/jmemstdex.c)

include_directories(${SOURCE_DIR})
include_directories(${THIRDPARTY_DIR}/stdex/include)

include_directories(${SOURCE_DIR})


add_library( ${PROJECT_NAME} STATIC ${jpeg_sources} )
