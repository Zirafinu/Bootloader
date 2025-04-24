
# foreach(_arg RANGE 3 ${CMAKE_ARGC})
#     message(STATUS "${_arg}: `${CMAKE_ARGV${_arg}}`")
# endforeach()

execute_process(COMMAND
openocd
 --file board/stm32f4discovery.cfg
 --command "init ; halt ; reset halt"
 --command "load_image ${CMAKE_ARGV3}"
#would be awesome if this could be done by openocd instead of handcoding it for a RAM link
 --command "dict set regs sp [read_memory 0x20000000 32 1] ; dict set regs pc [read_memory 0x20000004 32 1] ; set_reg $regs ; puts [get_reg {sp pc}]"
 --command "arm semihosting enable ; resume"
 RESULT_VARIABLE test_result
)
cmake_language(EXIT ${test_result})