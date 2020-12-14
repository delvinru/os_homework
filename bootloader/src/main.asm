;;; ===========================================================
;;; Programs that ask you for password and run 2 sector if pass was correct
;;; ===========================================================
    BITS 16
    org 0x7C00              ;'origin' of Boot code;

;;; ===========================================================
;;; Preparing for read 2 sector
;;; ===========================================================
    mov ax, 0x0000
    mov ds, ax              ;init data segment registry

    ;;Setup ES:BX memory addr
    mov bx, 0x1000          ;load sector to memory address 0x1000
    mov es, bx              ;ES=0x1000
    mov bx, 0               ;Offest in BX
    ;;Setup disk position
    mov ch, 0x00            ;cylinder 0
    mov cl, 0x02            ;starting sector to read from disk
    mov dh, 0x00            ;head 0
    mov dl, 0x00            ;drive 0; DL - drive number (0=A, 1=2nd floppy, 80h=drive0, 81h=drive1)

;;; ===========================================================
;;; Read ciphered program from disk into memory 0x1000
;;; ===========================================================
read_disk:
    mov ah, 0x02            ;AH - read disk sectors
    mov al, 0x01            ;Just read one sector
    int 0x13                ;BIOS interrupt for read disk
    jc read_disk            ;If carry flag == 1 try again

    mov ax, 0x03            ;Move cursor and shape to start screen
    int 0x10                ;BIOS interrupt for call video mode

    mov si, init_boot       ;Print welcome message
    call print_string
    
    mov si, input_message   ;Print input message
    call print_string

;;; ===========================================================
;;; Get user input and insert value in KEY
;;; ===========================================================
get_input:
    xor di, di              ;clear di
    mov di, KEY             ;now di point to KEY. KEY store key for decrypt 2 sector
keyloop:
    mov ah, 0x00            ;ah=0x00
    int 0x16                ;BIOS interrupt get keystroke, characherts goes into AL, scan code goes into AH

    inc BYTE [KEY_LEN]
    mov ah, 0x0E            ;for print user character
    cmp al, 0x0D            ;did user press 'enter' key?
    je successful_input

    int 0x10                ;print user character on screen
    mov [di], al            ;store user character in DI
    inc di                  ;increment di pointer for next character
    jmp keyloop             ;loop
;;; ===========================================================
;;  End user input
;;; ===========================================================

;;; ===========================================================
;;  Prepare new line
;;; ===========================================================
successful_input:
    mov BYTE [di], 0        ;last character of KEY==Null terminator - end of string
    dec BYTE [KEY_LEN]      ;Decrement value that get exactly size of input string
    mov si, new_line
    call print_string

;;; ===========================================================
;;; Start decrypt 2 sector with KEY
;;; ===========================================================
run_decode:
    ;; Init KSA
    mov ax, 0x2000
    mov es, ax
    mov bx, 0x00
    xor cx, cx

    mov si, KEY             ;SI store KEY for iteration
    ;;INIT array S in memory
init_loop:
    mov [ES:BX], cl         ;S[x] = x 
    inc cl
    inc bx
    cmp cl, 256
    je key_init_start
    jmp init_loop

;;; ===========================================================
;;; Create key loop
;;; ===========================================================
key_init_start:
    mov ax, 0x2000          ;pointer to array memory
    mov es, ax              ;ES=0x2000
    xor bx, bx              ;i = 0
    xor ax, ax              ;j = 0
    xor cx, cx              ;just for clear

key_loop:
    ; SI store KEY
    xor dx, dx
    mov dl, [ES:BX]         ;dl = S[i] 
    and dl, 0xFF            ;dl = dl % 256
    add al, dl              ;j = j+S[i]
    and al, 0xFF            ;j = j % 256

    cmp BYTE [si], 0x00      ;if end of KEY just reset SI
    je reset_si
    ;; Jump to this after reset si
after_check:
    add al, [si]            ;j = (j+key[i % 16])
    and al, 0xFF            ;j = j % 256

    ;; Swap values S[i] and S[j]
    push bx                 ;save register bx
    xor dx, dx              ;temporary index
    mov dl, [ES:BX]         ;dl = S[i]
    mov bl, al              ;offset for S[j]
    mov dh, [ES:BX]         ;dh = S[j]
    pop bx                  ;restore bx
    push bx
    mov [ES:BX], dh         ;S[i] = S[j]
    mov bl, al              ;offset for S[j]
    mov [ES:BX], dl         ;S[j] = S[i]
    pop bx
    ;; End swap
    
    inc si                  ;increase key postion 
    inc bx                  ;iteration
    cmp bx, 256             ;if bx==255 then our array was init and go to crypt
    je crypt
    jmp key_loop

reset_si:
    xor si, si
    mov si, KEY
    jmp after_check
;;; ===========================================================
;;; Finish create key loop
;;; ===========================================================

;;; ===========================================================
;;; Crypt loop
;;; ===========================================================
crypt:
    mov ax, 0x2000          ;ES=0x2000 - array location
    mov es, ax              ;ES=0x2000
    xor ax, ax              ;i = 0
    xor cx, cx              ;j = 0
    xor bx, bx              ;for el in data: - counter for data element

crypt_loop:
    ;; for el in data:      len(data) == 512
    inc al                  ;i = i + 1
    and al, 0xFF            ;i = i % 256
    ;; DONE ↑ 

    push bx                 ;save bx
    xor bx, bx              ;clear bx
    mov bl, al              ;offset for S[i] 
    add cl, [ES:BX]         ;j = j + S[i]
    and cl, 0xFF            ;j = j % 256
    pop bx                  ;restore bx

    ;; Swap values with stack
    push bx
    xor bx, bx              ;clear bx
    mov bl, al              ;offset for S[i]
    mov dl, [ES:BX]         ;dl = S[i]
    mov bl, cl              ;offset for S[j]
    mov dh, [ES:BX]         ;dh = S[j]
    mov bl, al              ;offset for S[i]
    mov [ES:BX], dh         ;S[i] = S[j]
    mov bl, cl              ;offset for S[j]
    mov [ES:BX], dl         ;S[j] = S[i]
    pop bx
    ;; End swap
    ;; DONE ↑ 

    push bx
    xor dx, dx              ;dx = 0; dx = S[i] + S[j]
    xor bx, bx              ;clear bx
    mov bl, al              ;set offset to S[i]
    mov dl, [ES:BX]         ;dl = S[i]
    mov bl, cl              ;set offset to S[j]
    add dl, [ES:BX]         ;dl = dl + S[j]
    and dl, 0xFF            ;dl = dl % 256
    pop bx                  ;restore bx

    push cx                 ;save i
    push ax                 ;save i
    push ES                 ;save ES=0x2000

    push bx                 ;save counter
    mov bl, dl              ;set offset to dx
    mov cl, [ES:BX]         ;cl store S[dx]
    pop bx                  ;restore counter

    mov ax, 0x1000          ;Put location of Ciphered block 0x1000
    mov es, ax              ;ES=0x1000
    xor [ES:BX], cl         ;xor value from memory and part of key, after put in memory location

    pop ES
    pop ax
    pop cx

    inc bx
    cmp bx, 512
    je run_second_sector
    jmp crypt_loop

;;; ===========================================================
;;; After decrypt call 2 sector
;;; ===========================================================
run_second_sector:
    ;; Load Program from memory location(0x1000)
    ;;reset segment registers for RAM
    mov ax, 0x1000          ;ciphered program location
    xor bx, bx              ;ES:BX = 0x1000:0x0000
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax 

    ;; Give control to 2 sector
    jmp 0x1000:0x0000

;;; ===========================================================
;;; Include helper for easy print string
;;; ===========================================================
%include 'src/print_string.asm'

;; Include this line for debug program and print_hex value
; %include 'src/print_hex.asm'

;;; ===========================================================
;;; Variable block
;;; ===========================================================
init_boot:     db 'Hello in CipheredOS!', 0xD, 0xA, 0
input_message: db 'Input password > ', 0
new_line:      db 0xA, 0xD, 0
KEY_LEN:       db 0
KEY:           db ''

;;; ===========================================================
;;; Boot magic
;;; ===========================================================
times 510-($-$$) db 0   ;pad file with zero byte
dw 0xaa55               ;Bios magic number