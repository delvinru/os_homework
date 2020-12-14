import random
from string import ascii_letters as al
from sys import argv


def crypt(data: bytes, key: bytes) -> bytes:
    """Implementation of RC4 algorithm"""
    # Init S
    S = [i for i in range(256)]

    # Run KSA algorithm
    j = 0
    for i in range(256):
        j = (j + S[i] + key[i % len(key)] ) % 256
        S[i], S[j] = S[j], S[i]

    # Compute keystram
    i = j = 0
    cipher = []
    for el in data:
        i = (i + 1) % 256           
        j = (j + S[i]) % 256        
        S[i], S[j] = S[j], S[i]     
        k = S[ (S[i]+S[j]) % 256 ] # element of key stream
        cipher.append(el ^ k) # just xor element of data and elemnt of key stream

    return b''.join([bytes([x]) for x in cipher])

def main() -> None:
    # Get file
    # fd = input('Введите имя файла: ')
    fd = 'bin/cipheredOS.bin'
    data = open(fd, 'rb').read()
    # Skip 1 sector and cipher only 2 sector

    # Generate key
    key = input('Input your password: ').encode()
    print('[+] Your secret key:', key.decode())
    # Compute cipher_text
    second_sector = data[512:]
    cipher_text = crypt(second_sector, key)

    # Write cipher text in file for next step
    os = b''.join([bytes([x]) for x in data[:512]]) + cipher_text
    # print(os)
    with open('cipheredOS.bin', 'wb') as f:
        for el in os:
            f.write(bytes([el]))
    
    print('[+] Cipher text written in file: `cipheredOS.bin`.')

if __name__ == "__main__":
    if len(argv) > 1:
        if argv[1] == '-h' or argv[1] == '--help':
            print('Usage:')
            print('\tInput file name, when program ask for you')
            print('\tCiphered file write in `ciphered`')
            exit(0)
    main()


# e9e9 21d8 d5c0 f87b 75dc ba95 3671 ff51
