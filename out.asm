section .text
global _start

_start:
    push rbp
    mov rbp, rsp
    sub rsp, 64

    mov rax, 5
    mov qword [rbp - 16], rax
    mov rax, qword [rbp - 16]
    mov qword [rbp - 8], rax
    mov rax, qword [rbp - 8]
    mov qword [rbp - 24], rax
    mov rax, 3
    mov qword [rbp - 32], rax
    mov rax, qword [rbp - 24]
    mov rbx, qword [rbp - 32]
    cmp rax, rbx
    setg al
    movzb rax, al
    mov qword [rbp - 40], rax
    mov rax, qword [rbp - 40]
    test rax, rax
    jz L0
    mov rax, 1
    mov qword [rbp - 48], rax
    mov rdi, qword [rbp - 48]
    mov rax, 60
    syscall
    jmp L1
L0:
    mov rax, 0
    mov qword [rbp - 56], rax
    mov rdi, qword [rbp - 56]
    mov rax, 60
    syscall
L1:

    ; Default exit 0
    mov rax, 60
    mov rdi, 0
    syscall
