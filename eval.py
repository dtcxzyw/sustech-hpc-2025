import subprocess
import os
import sys
import math

def normalize_output(output: str) -> str:
    return output.replace("-0.0000000000", "0.0000000000").strip()

data_dir = sys.argv[1]
program_path = sys.argv[2] # simulate.cpp

def get_answer(output_lines) -> str:
    for line in output_lines:
        if line.startswith("Final state:"):
            return line.strip()
    return "<invalid output>"

def get_timing(output_lines) -> float:
    for line in output_lines:
        if line.startswith("Time taken: "):
            return float(line[len("Time taken: "):].split()[0])
    return 1e9

subprocess.check_call(["icpx", "-std=c++17", "-xHost", "-qopenmp", "-O3", program_path, "/work/share/simulate/code-20250626/driver.o", "-o", "simulate"])

for test in os.listdir(data_dir):
    if not test.endswith(".in"):
        continue
    input_file = os.path.join(data_dir, test)
    output_file = os.path.join(data_dir, test[:-3] + ".out")
    out = get_answer(subprocess.check_output(["./simulate", input_file],timeout=120).decode().splitlines())
    with open(output_file) as f:
        ref = f.readlines()[0].strip()
    if normalize_output(out) != normalize_output(ref):
        print(f"Test {test} failed: expected {ref}, got {out}", flush=True)
        exit(1)

print("All tests passed.", flush=True)

perf_eval_file = "/work/share/simulate/input.bin"
perf_eval_out = "Final state: alpha = 0.000000000000 + -0.707106781187i, beta = 0.707106781187 + 0.000000000000i"
time_data = []
for i in range(10):
    out = subprocess.check_output(["./simulate", perf_eval_file],timeout=480).decode().splitlines()
    answer = get_answer(out)
    if normalize_output(answer) != normalize_output(perf_eval_out):
        print(f"Performance evaluation failed: expected {perf_eval_out}, got {answer}", flush=True)
        exit(1)
    time = get_timing(out)
    print(f"Round {i+1}: time = {time:.6f} ms", flush=True)
    time_data.append(time)

print("Time used for performance evaluation:", time_data, flush=True)
log_sum = sum(math.log(t) for t in time_data)
geomean = math.exp(log_sum / len(time_data)) / 1e3
print("Geomean:", geomean, "seconds", flush=True)

def rlerp(Lo, Hi, L, R, T):
    return Lo + (Hi - Lo) * (1/T - 1/R) / (1/L - 1/R)

if geomean > 281.56:
    score = 0
elif geomean > 8.66:
    score = rlerp(0, 60, 8.66, 281.56, geomean)
elif geomean > 5.71:
    score = rlerp(60, 80, 5.71, 8.66, geomean)
elif geomean > 4.68:
    score = rlerp(80, 90, 4.68, 5.71, geomean)
elif geomean > 1.19:
    score = rlerp(90, 100, 1.19, 4.68, geomean)
else:
    score = 100

print("Score:", score, flush=True)
with open("score.txt", "w") as f:
    f.write(f"{score:.2f}\n")
