.section .rodata
.align 2
data_run_mult:
    .float 2.0

.section .text  
.global run_enabled
.global run_hook
 
run_hook:
    b run_hook_exit
    lfs	0, 0x0094 (3)         # get float
    lis 12, run_enabled@ha      
    lwz 12, run_enabled@l (12)  
    cmpwi 12,0
    beq run_hook_exit
run_hook_modify:
    lis 12, data_run_mult@ha
    lfs 1, data_run_mult@l(12)
    fmuls 0,0,1
run_hook_exit: 
    b 0x0
