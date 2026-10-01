#![no_std]
#![allow(non_camel_case_types, non_snake_case, non_upper_case_globals)]

include!(concat!(env!("OUT_DIR"), "/common.rs"));

#[cfg(feature = "n64")]
pub mod n64 {
    use super::*;

    include!(concat!(env!("OUT_DIR"), "/n64.rs"));
}
