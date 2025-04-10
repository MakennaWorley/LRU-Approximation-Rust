#[derive(Clone)]
pub struct Page {
    pub number: i32,
    pub reference: bool,
}

pub struct Clock {
    capacity: usize,
    hand: usize,
    frames: Vec<Option<Page>>,
    page_faults: usize,
}

impl Clock {
    pub fn new(capacity: usize) -> Self {
        Self {
            capacity,
            hand: 0,
            frames: vec![None; capacity],
            page_faults: 0,
        }
    }

    pub fn insert(&mut self, page_number: i32) -> Option<i32> {
        for frame in &mut self.frames {
            if let Some(page) = frame {
                if page.number == page_number {
                    page.reference = true;
                    return None;
                }
            }
        }

        self.page_faults += 1;

        loop {
            match &mut self.frames[self.hand] {
                None => {
                    self.frames[self.hand] = Some(Page {
                        number: page_number,
                        reference: true,
                    });
                    self.advance_hand();
                    return None;
                }
                Some(page) => {
                    if !page.reference {
                        let evicted = page.number;
                        *page = Page {
                            number: page_number,
                            reference: true,
                        };
                        self.advance_hand();
                        return Some(evicted);
                    } else {
                        page.reference = false;
                        self.advance_hand();
                    }
                }
            }
        }
    }

    fn advance_hand(&mut self) {
        self.hand = (self.hand + 1) % self.capacity;
    }

    pub fn page_fault_count(&self) -> usize {
        self.page_faults
    }

    pub fn debug_state(&self) -> String {
        self.frames
            .iter()
            .enumerate()
            .map(|(i, page)| match page {
                Some(p) => {
                    let marker = if i == self.hand { "←" } else { " " };
                    format!("{marker}[{}|{}]", p.number, if p.reference { 1 } else { 0 })
                }
                None => {
                    let marker = if i == self.hand { "←" } else { " " };
                    format!("{marker}[   ]")
                }
            })
            .collect::<Vec<_>>()
            .join(" ")
    }
}
