# test driver for on target tests
execute_process(COMMAND
    openocd
    --file board/stm32f4discovery.cfg
    --command "init ; halt ; reset halt"
# flash the test binary
    --command "load_image ${CMAKE_ARGV3}"
# load the initial stack pointer and the Reset_Handler addresses
    --command "dict set regs sp [read_memory 0x20000000 32 1]"
    --command "dict set regs pc [read_memory 0x20000004 32 1]"
    --command "set_reg $regs"
    --command "puts [get_reg {sp pc}]"
# enable semi hosting and start the test
    --command "arm semihosting enable ; resume"
    RESULT_VARIABLE test_result
)

if(test_result EQUAL "0")
    message(VERBOSE "Test passed")
else()
    message(FATAL_ERROR "EXIT code : ${test_result} in binary '${CMAKE_ARGV3}'")
endif()
