set(SOURCE_FILES
    lapi.c
    lauxlib.c
    lcode.c
    lctype.c
    ldebug.c
    ldo.c
    ldump.c
    lfunc.c
    lgc.c
    llex.c
    lmem.c
    lobject.c
    lopcodes.c
    lparser.c
    lstate.c
    lstring.c
    ltable.c
    ltm.c
    lundump.c
    lvm.c
    lzio.c

    lbaselib.c
    lbitlib.c
    lcorolib.c
    ldblib.c
    liolib.c
    lmathlib.c
    loslib.c
    lstrlib.c
    ltablib.c
    lutf8lib.c
    loadlib.c
    linit.c)

set(HEADER_FILES
    lapi.h
    lauxlib.h
    lcode.h
    lctype.h
    ldebug.h
    ldo.h
    lfunc.h
    lgc.h
    llex.h
    llimits.h
    lmem.h
    lobject.h
    lopcodes.h
    lparser.h
    lprefix.h
    lstate.h
    lstring.h
    ltable.h
    ltm.h
    lua.h
    luaconf.h
    lundump.h
    lvm.h
    lzio.h

    lualib.h)

set_source_files_properties(${SOURCE_FILES} PROPERTIES LANGUAGE CXX)

add_library(Lua STATIC ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(Lua PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(Lua PRIVATE -DLUA_COMPAT_5_2)
if(WIN32)
    target_compile_definitions(Lua PRIVATE -D_CRT_SECURE_NO_WARNINGS)
elseif(APPLE)
    target_compile_definitions(Lua PRIVATE -DLUA_USE_MACOSX)
    target_link_libraries(Lua PRIVATE readline)
endif()
