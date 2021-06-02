import os 
from subprocess import call

print("\n============= TESTS =============");

print("\nTests Syntaxe OK :");
path = 'Tests/Syntaxe/OK/'
for filename in os.listdir(path):
    call(["./minicc", "-s", path+filename])
    print(filename);

print("\nTests Syntaxe KO :");
path = 'Tests/Syntaxe/KO/'
for filename in os.listdir(path):
    result = call(["./minicc", "-s", path+filename])
    print(filename);

print("\nTests Verif OK :");
path = 'Tests/Verif/OK/'
for filename in os.listdir(path):
    call(["./minicc", "-v", path+filename])
    print(filename);

print("\nTests Verif KO :");
path = 'Tests/Verif/KO/'
for filename in os.listdir(path):
    call(["./minicc", "-v", path+filename])
    print(filename);

print("\nTests Gencode OK :");
path = 'Tests/Gencode/OK/'
for filename in os.listdir(path):
    print(filename)
    call(["./minicc", path+filename])
    call(["./minicc_ref", path+filename,"-o", "ref.s"])
    stdout = open("res.txt", "w")
    stdout2 = open("ref.txt", "w")
    call(["java", "-jar", "../tools/Mars_4_2.jar", "out.s"], stdout=stdout)
    call(["java", "-jar", "../tools/Mars_4_2.jar", "ref.s"], stdout=stdout2)
    res = open("res.txt", "r")
    ref = open("ref.txt", "r")
    if res.read() == ref.read():
        print("===TEST OK===")
    else :
        print("###TEST KO###")
        exit()

print("\nTests Gencode KO :");
path = 'Tests/Gencode/KO/'
for filename in os.listdir(path):
    call(["./minicc", path+filename])
    print(filename);
