#!/bin/bash

# Exit immediately if any command fails
set -e

echo "Checking if Rust is installed..."
if ! command -v cargo &> /dev/null
then
    echo "Rust is not installed. Please install Rust first:"
    echo "https://doc.rust-lang.org/book/ch01-01-installation.html"
    exit 1
fi

echo "Pulling latest changes from main branch..."
git pull origin main

echo "Building the project with Cargo..."
cargo build

echo "Running the project..."
# You can comment/uncomment depending on whether you want terminal output or file output:

# To print into terminal:
cargo run -- BACKING_STORE.bin addresses.txt

# To save output to out.txt:
# cargo run -- BACKING_STORE.bin addresses.txt > out.txt
