with open("data.txt") as f:
    i = 1
    for row in f.read().splitlines():
        if not row:
            i += 1
            continue
        with open(f"post/post-{i:03d}.txt", "a") as g:
            g.write(row + "\r\n")
