import os
from jinja2 import Environment, FileSystemLoader, select_autoescape
import subprocess

base_dir = os.path.dirname(os.path.abspath(__file__))
env = Environment(loader=FileSystemLoader(base_dir), autoescape=select_autoescape())

states_str = """0 -4 -4 -4 -4
1 -4 -4 -4 4
2 -4 -4 4 -4
3 -4 -4 4 4
4 -4 4 -4 -4
5 -4 4 -4 4
6 -4 4 4 -4
7 -4 4 4 4
8 -2 -2 0 0
9 -2 0 -2 0
10 -2 0 0 -2
11 -2 0 0 2
12 -2 0 2 0
13 -2 2 0 0
14 -1 0 0 0
15 0 -2 -2 0
16 0 -2 0 -2
17 0 -2 0 2
18 0 -2 2 0
19 0 -1 0 0
20 0 0 -2 -2
21 0 0 -2 2
22 0 0 -1 0
23 0 0 0 -1
24 0 0 0 1
25 0 0 1 0
26 0 0 2 -2
27 0 0 2 2
28 0 1 0 0
29 0 2 -2 0
30 0 2 0 -2
31 0 2 0 2
32 0 2 2 0
33 1 0 0 0
34 2 -2 0 0
35 2 0 -2 0
36 2 0 0 -2
37 2 0 0 2
38 2 0 2 0
39 2 2 0 0
40 4 -4 -4 -4
41 4 -4 -4 4
42 4 -4 4 -4
43 4 -4 4 4
44 4 4 -4 -4
45 4 4 -4 4
46 4 4 4 -4
47 4 4 4 4
"""

mapping_str = """0 8 0 6 3 2
1 10 4 46 2 0
2 15 40 2 1 3
3 20 44 42 0 1
4 11 1 4 7 6
5 13 5 44 6 4
6 21 41 0 5 7
7 29 45 40 4 5
8 0 20 26 8 8
9 14 9 30 12 10
10 1 15 10 11 12
11 4 29 36 10 9
12 22 35 16 9 11
13 5 21 20 13 13
14 9 22 23 14 14
15 2 10 32 18 16
16 19 16 12 17 18
17 23 30 38 16 15
18 40 36 18 15 17
19 16 23 25 19 19
20 3 8 13 27 26
21 6 13 39 26 20
22 12 14 28 25 23
23 17 19 14 24 25
24 30 28 33 23 22
25 35 33 19 22 24
26 41 34 8 21 27
27 44 39 34 20 21
28 31 24 22 28 28
29 7 11 29 32 30
30 24 17 9 31 32
31 28 31 35 30 29
32 45 37 15 29 31
33 38 25 24 33 33
34 42 26 27 34 34
35 25 12 31 38 36
36 43 18 11 37 38
37 46 32 37 36 35
38 33 38 17 35 37
39 47 27 21 39 39
40 18 2 7 43 42
41 26 6 47 42 40
42 34 42 3 41 43
43 36 46 43 40 41
44 27 3 5 47 46
45 32 7 45 46 44
46 37 43 1 45 47
47 39 47 41 44 45
"""

fp_map = {
    0: "0.0",
    1: "1.0",
    -1: "-1.0",
    2: "0.70710678118654752440084436210485",
    -2: "-0.70710678118654752440084436210485",
    4: "0.5",
    -4: "-0.5",
}
states = [
    list(map(int, state.split())) for state in states_str.splitlines() if state.strip()
]
mappings = [
    list(map(int, line.split())) for line in mapping_str.splitlines() if line.strip()
]
template = env.get_template("./simulate_opt90.jinja")
with open(f"simulate_opt90.cpp", "w") as f:
    f.write(template.render(states=states, mappings=mappings, fp_map=fp_map))
subprocess.run(["clang-format", "-i", "simulate_opt90.cpp"])
template = env.get_template("./simulate_opt100.jinja")
with open(f"simulate_opt100.cpp", "w") as f:
    f.write(template.render(states=states, mappings=mappings, fp_map=fp_map))
subprocess.run(["clang-format", "-i", "simulate_opt100.cpp"])
