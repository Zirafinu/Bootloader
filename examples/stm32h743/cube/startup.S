
.syntax unified
.thumb
.cpu cortex-m4

  .section  .isr_vector
Stack_End_Data:
  .word estack
  .word Reset_Handler
  .word NMI_Handler
  .word Hard_fault_Handler
  .word Memeory_managment_fault_Handler
  .word Bus_fault_Handler
  .word Usage_fault_Handler
// place the reset handler into the same section as the ldr instruction
  .global Reset_Handler
  .weak  Reset_Handler
  .type  Reset_Handler, %function
Reset_Handler:
  // disbale interrupts
  cpsid i
  // enable ITCM and DTCM RAM are always on
  // initilize the stack pointer
  ldr sp, Stack_End_Data
  bl init_mem
  bl main
  b terminate_program
