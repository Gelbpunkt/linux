// SPDX-License-Identifier: GPL-2.0

//! Rust minimal sample.

use kernel::prelude::*;

module! {
    type: Gtx8,
    name: "gtx8rs",
    author: "Jens Reidel <adrian@travitia.xyz>",
    description: "Driver for GTx8 touchscreens",
    license: "GPL",
}

struct Gtx8 {
    numbers: Vec<i32>,
}

impl kernel::Module for Gtx8 {
    fn init(_module: &'static ThisModule) -> Result<Self> {
        pr_info!("Rust minimal sample (init)\n");
        pr_info!("Am I built-in? {}\n", !cfg!(MODULE));

        let mut numbers = Vec::new();
        numbers.try_push(72)?;
        numbers.try_push(108)?;
        numbers.try_push(200)?;

        Ok(Self { numbers })
    }
}

impl Drop for Gtx8 {
    fn drop(&mut self) {
        pr_info!("My numbers are {:?}\n", self.numbers);
        pr_info!("Rust minimal sample (exit)\n");
    }
}

