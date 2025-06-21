import sys
from jinja2 import Environment, FileSystemLoader, select_autoescape

env = Environment(loader=FileSystemLoader("."), autoescape=select_autoescape())

batch = int(sys.argv[1])
seqs = [""]

for i in range(batch):
    new_seqs = []
    for next in ["H", "X", "Y", "Z", "S"]:
        for seq in seqs:
            new_seqs.append(seq + next)
    seqs = new_seqs
print(seqs)
indexed_seqs = []
for seq in seqs:
    gates = []
    tag = 0
    for gate in seq:
        tag = tag * 5 + "HXYZS".index(gate)
        gates.append(gate)
    indexed_seqs.append((tag, gates))
template = env.get_template("./simulate_opt100.jinja")
with open(f"simulate_opt100_batch{batch}.cpp", "w") as f:
    f.write(template.render(seqs=indexed_seqs, step=batch))
