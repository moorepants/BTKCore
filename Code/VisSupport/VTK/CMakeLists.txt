SET(BTKVTK_SRCS
  btkVTKAxesWidget.cpp
  btkVTKDataObjectAdapter.cpp
  btkVTKForcePlatformsSource.cpp
  btkVTKGRFsFramesSource.cpp
  btkVTKInteractorStyleTrackballCamera.cpp
  btkVTKMarkersSource.cpp
  btkVTKMarkersFramesSource.cpp
  )

ADD_LIBRARY(BTKVTK ${BTK_LIBS_BUILD_TYPE} ${BTKVTK_SRCS})

TARGET_LINK_LIBRARIES(BTKVTK BTKCommon vtkCommon vtkFiltering vtkGraphics vtkRendering vtkHybrid)

IF(BTK_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(BTKVTK PROPERTIES ${BTK_LIBRARY_PROPERTIES})
ENDIF(BTK_LIBRARY_PROPERTIES)

IF(NOT BTK_INSTALL_NO_LIBRARIES)
  INSTALL(TARGETS BTKVTK
    RUNTIME DESTINATION ${BTK_INSTALL_BIN_DIR_CM24} COMPONENT RuntimeLibraries
    LIBRARY DESTINATION ${BTK_INSTALL_LIB_DIR_CM24} COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION ${BTK_INSTALL_LIB_DIR_CM24} COMPONENT Development)
ENDIF(NOT BTK_INSTALL_NO_LIBRARIES)

IF(NOT BTK_INSTALL_NO_DEVELOPMENT)
  FILE(GLOB __files "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
  INSTALL(FILES ${__files}
    DESTINATION ${BTK_INSTALL_INCLUDE_DIR_CM24}/BTKVTK
    COMPONENT Development)
ENDIF(NOT BTK_INSTALL_NO_DEVELOPMENT)