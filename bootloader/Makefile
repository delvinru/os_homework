.PHONY = run
main_file = main
print_my_name = print_name_group
output = cipheredOS

bochs: crypt
	# bochs.exe from windows
	bochs.exe -f config

qemu: crypt
	# qemu-system-x86_64.exe from windows
	# change this line on linux
	qemu-system-x86_64.exe -fda $(output).bin

crypt: build
	python3 rc4.py

debug: build
	python3 rc4.py
	bochsdbg.exe -f config

build:
	nasm -fbin src/$(main_file).asm -o bin/$(main_file)
	nasm -fbin src/$(print_my_name).asm -o bin/$(print_my_name)
	cat bin/$(main_file) bin/$(print_my_name) > bin/$(output).bin

clean:
	rm -rf bin/*
