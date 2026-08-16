import json

with open("docker/seccomp.json", "r") as f:
    data = json.load(f)

data["syscalls"][0]["names"].extend(["io_uring_setup", "io_uring_register", "io_uring_enter"])

with open("docker/seccomp.json", "w") as f:
    json.dump(data, f)