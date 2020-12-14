;;; ===========================================================
;;; Print hexadecimal string from DX register
;;; ===========================================================
;;; Ascii '0'-'9' = hex 0x30-0x39
;;; Ascii 'A'-'F' = hex 0x41-0x46
;;; Ascii 'a'-'f' = hex 0x61-0x66

print_hex:
    pusha               ;save all registers to the stack
    xor cx, cx          ;init loop counter

hex_loop:
    cmp cx, 4           ;end of the loop?
    je end_hex_loop

    ;; Convert DX hex values to ASCII
    mov ax, dx
    and ax, 0x000F      
    add al, 0x30        ;get ascii number value
    cmp al, 0x39
    jle move_intoBX
    add al, 0x7         ;to get ascii 'A'-'F'

move_intoBX:
    mov bx, hexString + 5 ;base address of hexString + length of string
    sub bx, cx          ;subtract loop counter
    mov [bx], al        
    ror dx, 4           ;rotate right by 4 bits

    inc cx
    jmp hex_loop        ;loop for next digit in dx

end_hex_loop:
    mov si, hexString
    call print_string

    popa                ;restore all register
    ret                 ;return to caller

;; Data
hexString:  db '0x0000', 0xA, 0xD, 0