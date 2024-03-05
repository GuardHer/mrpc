# 1 - 1000000 
import math
def check(number):
    r = int(math.sqrt(number))
    for i in range(2, r):
        if number % i == 0:
            return False
        
    return True

def get_sum(str_num: str):
    res = 0
    for i in str_num:
        res += int(i)
        
    return res == 23

ret = 0
for i in range(1, 1000001):
    if check(i) and get_sum(str(i)):
        ret += 1
        
print(ret)