def add(a, b):
    return a+b

def operate(func, a, b):
    return func(a, b)

print(operate(add, 2, 3))
