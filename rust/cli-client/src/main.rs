use std::io::{self, Write};

fn main() {
    loop {
        print!("> ");
        io::stdout().flush().unwrap();

        let mut input = String::new();
        io::stdin()
            .read_line(&mut input)
            .expect("Failed to read command");

        input = input.trim().to_string();

        // Reverse the string
        let reversed = reverse_string(&input);

        // Output the reversed string
        println!("Reversed string: {}", reversed);
    }
}

fn reverse_string(s: &str) -> String {
    s.chars().rev().collect()
}
