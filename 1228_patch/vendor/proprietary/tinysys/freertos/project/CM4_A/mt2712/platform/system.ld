/* Entry Point */
ENTRY(Reset_Handler)

_SCP_IPC_SHARE_BUFFER_ADDR = 0x1FDC0;
/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size = 0x0;      /* required amount of heap  */
_Min_Stack_Size = 0x800; /* required amount of stack for MSP*/
/* Specify the memory areas */
MEMORY
{
  RTOS   (rwx)                : ORIGIN = 0x00000000, LENGTH = 127K
  DRAM_HEAP   (rw)            : ORIGIN = 0xBE900000, LENGTH = 2048K
  DRAM_DATA   (rw)            : ORIGIN = 0xBED00000, LENGTH = 2048K
}
__RTOS_segment_start__ = ORIGIN( RTOS );
__RTOS_segment_end__ = __RTOS_segment_start__ + LENGTH( RTOS );

/* Define output sections */
SECTIONS
{
  /* The startup code goes first into RTOS */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >RTOS
  .bl :
  {
    BL = ABSOLUTE(.);
    KEEP (*(.blvec) ) ;
    *(.bltext) *(.bltext.*) ;
    *(.blrodata) *(.blrodata.*) ;
    . = ALIGN(4);
    __pubkeys_start = ABSOLUTE(.);
    KEEP (*(.pubkeys) ) ;
    __pubkeys_end = ABSOLUTE(.);
    . = ALIGN(4);
  } >RTOS
  .eedata :
  {

    . = ALIGN(4);
    __eedata_start = ABSOLUTE(.);
     KEEP (*(.eedata) ) ;
    __eedata_end = ABSOLUTE(.);
 /*   . = LENGTH(eedata) - 1; /* make sure it is all full of 0xFFs */
      BYTE (0xff) ;
  } >RTOS = 0xff
  /* The program code and other data goes into  RTOS*/
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
	. = ALIGN(8);
	__internal_app_start = ABSOLUTE(.);
	KEEP (*(.internal_app_init) ) ;
	__internal_app_end = ABSOLUTE(.);
	. = ALIGN(4);
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
	*(.eh_frame)
	 __commands_start = .;
    KEEP (*(.commands))
    __commands_end = .;


    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
    _exit = .;
  } >RTOS


   .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >RTOS
    .ARM : {
    __exidx_start = .;
      *(.ARM.exidx*)
      __exidx_end = .;
    } >RTOS

  .preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >RTOS
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >RTOS
  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(.fini_array*))
    KEEP (*(SORT(.fini_array.*)))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >RTOS

  /* used by the startup to initialize data */
  _sidata = .;

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data :
  {
    . = ALIGN(4);
	__data_start = ABSOLUTE(.);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */

    . = ALIGN(4);

    _edata = .;        /* define a global symbol at data end */
	__data_end = ABSOLUTE(.);
  } >RTOS

  /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >RTOS

  /* kernel_heap_stack section, used to check that there is enough RAM left */
  ._kernel_heap_stack :
  {
    . = ALIGN(4);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
    _estack = .; /* Highest address of the MSP stack */
  } >RTOS

  ._dram_heap (NOLOAD):
  {
    _dram_heap_start = .;
    *(.dram_heap);
    _dram_heap_end = .;
  } >DRAM_HEAP

  ._dram_data (NOLOAD):
  {
    _dram_data_start = .;
    *(.dram_data);
    _dram_data_end = .;
  } >DRAM_DATA

    . = ALIGN(4);
  .share (NOLOAD) :
  {
    _sshare = .;
    *(.share)
    . = ALIGN(4);
    _eshare = .;
  } >RTOS
  __data_data = LOADADDR(.data);

   .ipc_send_buffer 0x0001FDC0 (NOLOAD) :
  {
    __ipc_send_buffer_start = .;
    KEEP (*(.ipc_send_buffer))
    __ipc_send_buffer_end = .;
  } >RTOS

   .ipc_rcv_buffer 0x0001FEE0 (NOLOAD) :
  {
    __ipc_rcv_buffer_start = .;
    KEEP (*(.ipc_rcv_buffer))
    __ipc_rcv_buffer_end = .;
  } >RTOS

  _s_ipc_share = _SCP_IPC_SHARE_BUFFER_ADDR;
  ASSERT(_eshare < _s_ipc_share, "The share buffer is over ipc_buffer!!")
}

__stack_top = 0;
__shared_start = _SCP_IPC_SHARE_BUFFER_ADDR;
__shared_end = _SCP_IPC_SHARE_BUFFER_ADDR;
__app_start = __data_data + __data_end - __data_start;
