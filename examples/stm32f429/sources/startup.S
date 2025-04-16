
.syntax unified
.thumb
.cpu cortex-m4

  .section  .isr_vector
Stack_End_Data:
  .word estack
  .word Reset_Handler
// place the reset ahndler into the same section as the ldr instruction
  .global Reset_Handler
  .weak  Reset_Handler
  .type  Reset_Handler, %function
Reset_Handler:
  ldr sp, Stack_End_Data
  bl  init_mem
 // just fall through to init_mem, has to be ensured by the linker
