add_executable(physics physics.cpp)
target_link_libraries(physics PRIVATE quadtree)
setWarnings(physics)
setStandard(physics)
# Profiling
target_compile_options(physics PRIVATE -pg -O1)
target_link_libraries(physics PRIVATE -pg)