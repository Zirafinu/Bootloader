
.syntax unified
.thumb
  .global Reset_Handler
  .section  .text.Reset_Handler
  .weak  Reset_Handler
  .type  Reset_Handler, %function
Reset_Handler:
  ldr r0, =estack
  bl  init_mem

.size  Reset_Handler, .-Reset_Handler

