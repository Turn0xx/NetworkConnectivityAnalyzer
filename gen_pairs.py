import random

# max node number
ub = 999

for i in range(1000):
    src = random.randint(0, ub)
    dst = random.randint(0, ub)
    print(f"{src}\t{dst}")

