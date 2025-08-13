# 🗄️ MiniDB — A Tiny SQLite-like Database in C

This repository contains a low-level C implementation of a **simple relational database** engine, inspired by SQLite.  
It supports:
- Basic `INSERT` and `SELECT` statements
- Row storage using a **B-Tree** structure
- Paging & disk persistence
- A minimal REPL (Read-Eval-Print Loop) with meta commands

The project is purely educational — a way to explore **databases from scratch** and **systems programming concepts** like memory management, file I/O, and binary data layout.

---


---

## 🚀 Features

- **Persistent storage** — all data is written to a file you specify when launching.
- **B-tree** indexing — enables efficient lookups and inserts.
- **Fixed-size row layout** — manual serialization & deserialization.
- **Meta commands**:
  - `.exit` — save and quit
  - `.btree` — print the B-tree structure
  - `.constants` — print database constants

---

## 🔧 Requirements

You’ll need:
- **GCC** or another C compiler
- **Make** (optional but useful)
- A Unix-like environment (Linux/macOS, WSL on Windows)

For Ubuntu/Debian:
```bash
sudo apt update
sudo apt install build-essential
```
How to run it:
Create a database file : fileName.db



