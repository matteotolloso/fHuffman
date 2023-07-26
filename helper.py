# gnerate a file with 1 billion ASCII characters

import random
import string

def generate_file():
    with open('dataset/8mb.txt', 'w') as f:
        chars = []
        for i in range(int(2**24)):
            c = random.choice(string.ascii_letters)  
            chars.append(c)
        f.write(''.join(chars))
        



if __name__ == '__main__':
    generate_file()