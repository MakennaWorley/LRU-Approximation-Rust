# LRU Approximation
Extension of homework 5 recoded in Rust

This uses rust code to mimic LRU Approximation with Page Replacement.

To run:

Make sure you have Rust installed on your device (https://doc.rust-lang.org/beta/book/ch01-01-installation.html)

Pull down the lastest changes from the main branch.

Run ```cargo build``` to compile the project.

Then run from that same location ```cargo run -- BACKING_STORE.bin addresses.txt``` to have the output printed into the terminal (this can take a while)
or ```cargo run -- BACKING_STORE.bin addresses.txt > out.txt``` to have the summary written to the 'out.txt' file.