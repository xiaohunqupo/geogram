set(APP_NAME hexdom_pipeline)

aux_source_directories(SOURCES "" .)
vor_add_executable(${APP_NAME} ${SOURCES})
target_link_libraries(${APP_NAME} exploragram geogram)

set_target_properties(${APP_NAME} PROPERTIES FOLDER "GEOGRAM/Examples")
