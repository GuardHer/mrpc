
res = 0

for i in range(0, 101):
    for j in range(0, 101):
        if j - i >= 10:
            res += 1

print(res)