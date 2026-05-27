# wvsh

A minimal Unix shell built from scratch in C, as a hands-on study of low-level systems programming — process creation, memory management, and the Unix execution model.

> This project is actively growing. New features are added incrementally as the codebase matures.

---

## How It Works

Every command you type goes through three steps:

1. **Read** — `wvsh_read_line()` pulls raw input from stdin into a heap-allocated buffer
2. **Tokenize** — `wvsh_tokenize_line()` splits the input into a `char**` array on whitespace delimiters
3. **Execute** — `wvsh_execute_command()` either calls a built-in directly, or `fork()`s a child process and hands it off to `execvp()`

---

## Built-in Commands

| Command | Description |
|---|---|
| `cd [path]` | Change working directory (defaults to `$HOME`) |
| `help` | List all built-in commands |
| `exit` | Exit the shell |

---

## Compilation & Usage

**Default build** (manual buffer reader):
```bash
gcc -o wvsh wvsh.c
./wvsh
```

**Standard library build** (uses `getline`):
```bash
gcc -DWVSH_USE_STD_GETLINE -o wvsh wvsh.c
./wvsh
```

**With debug symbols:**
```bash
gcc -g -Wall -Wextra -o wvsh wvsh.c
```

---

## Roadmap

Features planned or in progress:

- [ ] Pipes (`cmd1 | cmd2`)
- [ ] I/O redirection (`>`, `<`, `>>`)
- [ ] Background job execution (`&`)
- [ ] Signal handling (`Ctrl+C`, `Ctrl+Z`)
- [ ] Command history
- [ ] Environment variable expansion

---

## Contributing

Contributions are welcome at any stage — whether it's a bug fix, a new built-in, or a feature from the roadmap above.

1. Fork the repository
2. Create a branch: `git checkout -b feature/your-feature-name`
3. Commit your changes: `git commit -m "Add: your feature description"`
4. Push and open a Pull Request

Please keep code style consistent with the existing source — clear comments, explicit error handling, and no unnecessary dependencies.

---

## License

Public domain under [The Unlicense](https://unlicense.org) — free to use, copy, modify, and distribute for any purpose.
