mod clock;
use clock::Clock as LRU;
use atty::Stream;

use std::{
    collections::HashMap,
    env,
    fs::File,
    io::{self, BufRead, BufReader, Read, Seek},
    process,
};

const FRAME_COUNT: usize = 128;
const PAGE_TABLE_SIZE: usize = 256;
const TLB_SIZE: usize = 16;
const CHUNK: usize = 256;

struct TLB {
    page_numbers: [i32; TLB_SIZE],
    frame_numbers: [i32; TLB_SIZE],
    index: usize,
}

impl TLB {
    fn new() -> Self {
        Self {
            page_numbers: [-1; TLB_SIZE],
            frame_numbers: [-1; TLB_SIZE],
            index: 0,
        }
    }

    fn search(&self, page_number: i32) -> Option<i32> {
        self.page_numbers
            .iter()
            .zip(self.frame_numbers.iter())
            .find(|&(p, _)| *p == page_number)
            .map(|(_, &f)| f)
    }

    fn add(&mut self, page_number: i32, frame_number: i32) {
        self.page_numbers[self.index] = page_number;
        self.frame_numbers[self.index] = frame_number;
        self.index = (self.index + 1) % TLB_SIZE;
    }
}

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() != 3 {
        eprintln!("Usage: vm_lru <backing_store> <input_file>");
        process::exit(1);
    }

    let backing_store_path = &args[1];
    let input_path = &args[2];

    let mut backing_store = File::open(backing_store_path)
        .unwrap_or_else(|_| panic!("Could not open backing store: {}", backing_store_path));

    let input_file = BufReader::new(File::open(input_path)
        .unwrap_or_else(|_| panic!("Could not open input file: {}", input_path)));

    let mut page_table: HashMap<i32, usize> = HashMap::new();
    let mut page_table_valid: [bool; PAGE_TABLE_SIZE] = [false; PAGE_TABLE_SIZE];
    let mut physical_memory = vec![vec![0i8; CHUNK]; FRAME_COUNT];
    let mut next_free_frame = 0;
    let mut lru = LRU::new(FRAME_COUNT);
    let mut tlb = TLB::new();

    let mut translated_count = 0;
    let mut tlb_hits = 0;

    let output_is_terminal = atty::is(Stream::Stdout);

    for line in input_file.lines() {
        let logical_address: i32 = match line {
            Ok(val) => val.trim().parse().unwrap_or(-1),
            Err(_) => continue,
        };

        if logical_address == -1 {
            continue;
        }

        let page_number = (logical_address >> 8) & 0xFF;
        let offset = logical_address & 0xFF;
        let mut frame_number: i32;

        if let Some(frame) = tlb.search(page_number) {
            tlb_hits += 1;
            frame_number = frame;
        } else {
            if !page_table_valid[page_number as usize] {
                let mut buffer = [0i8; CHUNK];
                let seek_pos = (page_number * CHUNK as i32) as u64;

                backing_store
                    .seek(io::SeekFrom::Start(seek_pos))
                    .expect("Seek failed");

                backing_store
                    .read_exact(unsafe {
                        std::mem::transmute::<&mut [i8], &mut [u8]>(&mut buffer)
                    })
                    .expect("Read failed");

                let evicted_page = lru.insert(page_number);

                if next_free_frame < FRAME_COUNT {
                    frame_number = next_free_frame as i32;
                    next_free_frame += 1;
                } else {
                    if let Some(evicted) = evicted_page {
                        frame_number = *page_table.get(&evicted).unwrap() as i32;
                        page_table_valid[evicted as usize] = false;
                        page_table.remove(&evicted);
                    } else {
                        panic!("Frame full but no page to evict!");
                    }
                }

                physical_memory[frame_number as usize].copy_from_slice(&buffer);
                page_table.insert(page_number, frame_number as usize);
                page_table_valid[page_number as usize] = true;
            }

            frame_number = *page_table.get(&page_number).unwrap() as i32;
            tlb.add(page_number, frame_number);
        }

        let value = physical_memory[frame_number as usize][offset as usize];
        let physical_address = frame_number * CHUNK as i32 + offset;

        if output_is_terminal {
            println!(
                "Virtual address: {} Physical address: {} Value: {}",
                logical_address, physical_address, value
            );
            println!("Memory: {}", lru.debug_state());

            println!("Memory: {}", lru.debug_state());
            // std::thread::sleep(std::time::Duration::from_millis(100));
        } else {
            println!(
                "Virtual address: {} Physical address: {} Value: {}",
                logical_address, physical_address, value
            );
        }

        translated_count += 1;
    }

    println!("Number of Translated Addresses = {}", translated_count);
    println!("Page Faults = {}", lru.page_fault_count());
    println!(
        "Page Fault Rate = {:.3}",
        lru.page_fault_count() as f32 / translated_count as f32
    );
    println!("TLB Hits = {}", tlb_hits);
    println!(
        "TLB Hit Rate = {:.3}",
        tlb_hits as f32 / translated_count as f32
    );
}
