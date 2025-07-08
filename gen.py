import subprocess
import random

def generate(idx):
    size = 48 * 8 * random.randint(1, 10000)
    subprocess.check_call(["./gen", str(size), f"data/data_{idx}.in"])
    out = subprocess.check_output(["./simulate_ref", f"data/data_{idx}.in"]).decode()
    with open(f"data/data_{idx}.out", "w") as f:
        f.write(out)

for i in range(1000):
    generate(i)
