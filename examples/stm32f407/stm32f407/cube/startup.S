
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

  // enable CCM RAM
  ldr r0, RCC_AHB1_ENR
  ldr r1, [r0]
  orr r1, r1, 0x00100000
  str r1, [r0]
  // initilize the stack pointer
  ldr sp, Stack_End_Data
  bl init_mem
  bl main
  b terminate_program
RCC_AHB1_ENR:
  .word 0x40023830
