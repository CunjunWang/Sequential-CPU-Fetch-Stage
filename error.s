0x000: 03                   | 	halt
0x010:                      | .align 0x10
0x010: 1e                   | 	nop
0x020:                      | .align 0x20
0x020: 6800                 | 	addq  %rax, %rax
0x030:                      | .align 0x30
0x030: 6901                 | 	addq  %rax, %rcx
0x040:                      | .align 0x40
0x040: 6a02                 | 	addq  %rax, %rdx
0x050:                      | .align 0x50
0x050: 6b03                 | 	addq  %rax, %rbx
0x060:                      | .align 0x60
0x060: 7cadddda0000000000   | 	jle  0xdaddad
0x070:                      | .align 0x70
0x070: 7ddaddda0000000000   | 	jl   0xdaddda
0x080:                      | .align 0x80
0x080: 77adddda0000000000   | 	je   0xdaddad
0x090:                      | .align 0x90
0x090: 7aadddda0000000000   | 	jne  0xdaddad
0x100:                      | .align 0x100
0x100: 78dddda0000000000   | 	jge  0xdaddad
0x110:                      | .align 0x110
0x110: 79adddda0000000000   | 	jg   0xdaddad
0x120:                      | .align 0x120	
0x120: 2700                 | 	cmovl  %rax, %rax
0x130:                      | .align 0x130
0x130: 2a01                 | 	cmovl  %rax, %rcx
0x140:                      | .align 0x140
0x140: 2b02                 | 	cmovl  %rax, %rdx
0x150:                      | .align 0x150
0x150: 9e                   | 	ret
0x160:                      | .align 0x160
0x160: 82634129078563412   | 	call    0x1234567890123456
                            | 	
                            | 
                            | 
                            | 
                            | 
                            | 	
