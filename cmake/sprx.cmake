add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${PRX_STRIP} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/alear.prx -o ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/alear.prx.stripped
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${MAKE_FSELF} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/alear.prx.stripped ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/alear.sprx
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND wsl --exec rm -f bin/alear.pahole.h
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND wsl --exec pahole --compile -a -A -d -I bin/alear.prx > bin/alear.pahole.h
)

# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#     COMMAND wsl --exec abi-dumper  bin/alear.prx -o bin/alear.abi.dump
# )


add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/alear.sprx"
	"E:/emu/rpcs3/dev_hdd0/game/LBP1DEBUG/USRDIR/alear.sprx")