;;; ===========================================================
;;; Just print username and group
;;; ===========================================================

;;; Because this is just 'module' we don't need BIOS magic number and 510 in padding
    mov si, welcome
    call print_string

    mov si, author          
    call print_string

    mov si, group
    call print_string

    cli                     ;clear interrupts instead of 'jmp $'
    hlt                     ;halt the cpu
    ;; Include File with print function
    %include 'src/print_string.asm'

;;; Variable block
welcome: db 0xA, 0xD, 'Congrutulations, cipheredOS was succefully loaded!', 0xA, 0xD, 0
author:  db 'Kolesnikov Alexey', 0xA, 0xD, 0
group:   db 'KKSO-01-19', 0xA, 0xD, 0

times 512 - ($-$$) db 0