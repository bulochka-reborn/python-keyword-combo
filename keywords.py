import builtins
import keyword
import json


def printsl(st):
    print(st, end="")

all_words = [k for k, v in vars(builtins).items() if k[0].islower()]


printsl("{")
for word in all_words:
    c = 0
    printsl("{")
    for letter in range(len(word)):
        printsl(f"'{word[letter]}'")
        printsl(", ")

    while letter < 12:
        printsl("'\\0'")
        if letter < 11:
            printsl(", ")
        letter += 1
    if word != all_words[-1]:
        printsl("}, ")
    else:
        printsl("}")


printsl("}")

    

