# gnerate a file with 1 billion ASCII characters

import random
import string

def generate_file():
    with open('dataset/bigdata.txt', 'w') as f:
        chars = []
        for i in range(int(1e9)):
            c = random.choice(string.ascii_letters)  
            chars.append(c)
        f.write(''.join(chars))
        



if __name__ == '__main__':
    generate_file()