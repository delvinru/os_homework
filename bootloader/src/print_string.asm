;;; ===========================================================
;;; Helpful functions that just print string from SI register
;;; ===========================================================

;; Set video mode
mov ah, 0x00            ;int 0x10/ ah 0x00 = set video mode
mov al, 0x08            ;160x200 text mode
int 0x10

;Change color/Palette
mov ah, 0x0B
mov bh, 0x00
mov bl, 0x00            ;set color to BLACK
int 0x10

print_string:
    mov ah, 0x0E            ;BIOS interrupt for write text in teletype mode
    mov bh, 0x00            ;page number

print_char:
    lodsb
    cmp al, 0               ;compare that not end of string
    je end_print            ;jmp if equal(al=0) to end print
    int 0x10                ;BIOS interrupt for print character
    jmp print_char          ;loop

end_print:
    ret