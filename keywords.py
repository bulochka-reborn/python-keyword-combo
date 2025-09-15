import builtins
import keyword
import inspect
import json


def printsl(st):
    print(st, end="")


all_words = []

for obj_str, obj in vars(builtins).items():
    if not obj_str in all_words:
        all_words.append(obj_str)

    for attr_str in dir(obj):
        if not attr_str in all_words:
            all_words.append(attr_str)

word_ln = [len(i) for i in all_words]

print(max(word_ln))

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