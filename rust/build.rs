use std::path::{Path, PathBuf};

#[derive(Debug)]
struct CommonTraits;

impl bindgen::callbacks::ParseCallbacks for CommonTraits {
    fn blocklisted_type_implements_trait(
        &self,
        name: &str,
        derived: bindgen::callbacks::DeriveTrait,
    ) -> Option<bindgen::callbacks::ImplementsTrait> {
        use bindgen::callbacks::{DeriveTrait, ImplementsTrait};

        (name.starts_with("ModLoader_")
            && matches!(
                derived,
                DeriveTrait::Copy | DeriveTrait::Debug | DeriveTrait::Default
            ))
        .then_some(ImplementsTrait::Yes)
    }
}

fn bindings(include: &Path) -> bindgen::Builder {
    bindgen::Builder::default()
        .use_core()
        .derive_default(true)
        .layout_tests(false)
        .formatter(bindgen::Formatter::Prettyplease)
        .disable_header_comment()
        .prepend_enum_name(false)
        .translate_enum_integer_types(true)
        .clang_args(["-x", "c", "-std=c11", "-ffreestanding"])
        .clang_arg(format!("-I{}", include.display()))
        .parse_callbacks(Box::new(bindgen::CargoCallbacks::new()))
}

fn generate(builder: bindgen::Builder, output: &Path) {
    builder
        .generate()
        .expect("Unable to generate ModLoader ABI bindings")
        .write_to_file(output)
        .expect("Unable to write ModLoader ABI bindings");
}

fn main() {
    let manifest = PathBuf::from(std::env::var_os("CARGO_MANIFEST_DIR").unwrap());
    let include = manifest.join("../include");
    let output = PathBuf::from(std::env::var_os("OUT_DIR").unwrap());
    let mut common = bindings(&include)
        .allowlist_type("ModLoader_.*")
        .allowlist_var("MODLOADER_.*");

    for header in [
        "modloader_debug.h",
        "modloader_events.h",
        "modloader_log.h",
        "modloader_platform.h",
        "modloader_settings.h",
        "modloader_video.h",
    ] {
        common = common.header(include.join(header).to_string_lossy());
    }
    if std::env::var_os("CARGO_FEATURE_ADAPTER").is_some() {
        common = common.header(include.join("modloader_adapter_api.h").to_string_lossy());
    }
    if std::env::var_os("CARGO_FEATURE_MODULE").is_some() {
        for header in [
            "modloader_module_exports.h",
            "modloader_lobby.h",
            "modloader_rml.h",
            "modloader_libc.h",
        ] {
            common = common.header(include.join(header).to_string_lossy());
        }
    }
    generate(common, &output.join("common.rs"));

    if std::env::var_os("CARGO_FEATURE_N64").is_some() {
        let mut n64 = bindings(&include)
            .allowlist_type("ModLoader_N64_.*")
            .allowlist_var("MODLOADER_(N64|EVENT_N64)_.*")
            .allowlist_recursively(false)
            .parse_callbacks(Box::new(CommonTraits));
        for header in [
            "n64/modloader_n64_adapter.h",
            "n64/modloader_n64_module.h",
            "n64/modloader_n64_patch.h",
        ] {
            n64 = n64.header(include.join(header).to_string_lossy());
        }
        generate(n64, &output.join("n64.rs"));
    }
}
