.section .text  

/*
C21B8F60 00000005
C0030094 81DF03D8
55CE05AD 41820014
3DC04000 91C1FFFC
C081FFFC EC000132
00000000 00000000
*/

.global run_hook
.global is_run_enabled
.global run_speed_mult

run_hook:
    .long 0
    b run_hook_epilogue
# orig instruction
    lfs	0, 0x0094 (3)         # get float
# check if enabled
    lis 12, is_run_enabled@ha      
    lwz 12, is_run_enabled@l (12)  
    cmpwi 12,0
    beq run_hook_exit
# check if holding B
    lwz 12, 0x3D8 (31)
    rlwinm. 0, 12, 0, 0x200
    beq run_hook_exit
run_hook_modify:
    lis 12, run_speed_mult@ha
    lfs 1, run_speed_mult@l(12)
    fmuls 0,0,1
run_hook_exit: 
    b 0x8                       # skip orig instruction
run_hook_epilogue:
    b 0x0
    .long 0x801b8f64
