message("* qt/pc9801")

set(s_qt_pc9801_headers
	  menuclasses.h
)
if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_pc9801_headers_MOC ${s_qt_pc9801_headers})
else()
  QT4_WRAP_CPP(s_qt_pc9801_headers_MOC ${s_qt_pc9801_headers})
endif()
add_library(qt_pc9801
	  MainWindow.cpp
	  ${s_qt_pc9801_headers_MOC}
)


