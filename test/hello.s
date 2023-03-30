	.file	"hello.c"
	.text
	.globl	message
	.data
	.align 4
	.type	message, @object
	.size	message, 14
message:
	.string	"hello world!\n"
	.comm	buf,1024,32
	.text
	.globl	main
	.type	main, @function
main:
	endbr32
	pushl	$message
	call	printf
	addl	$4, %esp
	movl	$0, %eax
	ret
	.size	main, .-main
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 4
	.long	 1f - 0f
	.long	 4f - 1f
	.long	 5
0:
	.string	 "GNU"
1:
	.align 4
	.long	 0xc0000002
	.long	 3f - 2f
2:
	.long	 0x3
3:
	.align 4
4:
