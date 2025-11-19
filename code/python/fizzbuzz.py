import sys
def isPrime(num):
    primes = [ 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97 ];
    
    for i in primes:
        if (num == i):
            return True;
    return False;

tried = 0;
num = 0;
CountPrimes = 0; 
SumEven = 0;
MaxOdd = 0;
SumDivBy7 = 0;

print("Enter a number between 10 and 200: ", end='');
while (tried < 3):
    num = input();
    if (isinstance(num, int)):
        print("Enter an integer please");
        tried+=1;
    else:
        num = int(num);
        if (num < 10) or (num > 200):
            print("Invalid range. Input again:", end='');
            tried+=1;
            continue;
        break;
    
if (tried == 3):
    print("");
    print("Too many attempts exit.");
    sys.exit();

for i in range(1, num+1):
    if(isPrime(i)):
        print(f"[{i}] ", end='');
        CountPrimes+=1;

    elif(((i % 3) == 0) and ((i % 5) == 0)):
        print("FizzBuzz ", end='');

    elif((i % 3) == 0):
        print("Fizz ", end='');

    elif((i % 5) == 0):
        print("Buzz ", end='');

    if((i % 7) == 0):
        SumDivBy7+=i;

    if((i % 2) == 0):
        SumEven+=i;
        print(f"{i} ", end='');

    else:
        MaxOdd = i if (i > MaxOdd) else MaxOdd;
        print(f"{i} ", end='');

print();
print(f"Count primes: {CountPrimes}");
print(f"Sum Even: {SumEven}");
print(f"Max Odd: {MaxOdd}");
print(f"Sum Div by 7: {SumDivBy7}");
