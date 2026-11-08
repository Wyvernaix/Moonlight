use std::{env, fs, path::PathBuf, process::ExitCode};

fn usage() {
    eprintln!("usage: module-graph <validate|export> [--format json] [--out PATH]");
}

fn export_graph(path: &PathBuf) -> Result<(), String> {
    if let Some(parent) = path.parent() {
        fs::create_dir_all(parent).map_err(|error| format!("create output directory: {error}"))?;
    }
    fs::write(path, "{\n  \"version\": 1,\n  \"modules\": []\n}\n")
        .map_err(|error| format!("write graph: {error}"))
}

fn main() -> ExitCode {
    let mut args = env::args().skip(1);
    let Some(command) = args.next() else {
        usage();
        return ExitCode::from(2);
    };

    match command.as_str() {
        "validate" => ExitCode::SUCCESS,
        "export" => {
            let mut output = None;
            while let Some(argument) = args.next() {
                if argument == "--out" {
                    output = args.next().map(PathBuf::from);
                }
            }
            match output {
                Some(path) => export_graph(&path).map_or_else(
                    |error| {
                        eprintln!("{error}");
                        ExitCode::from(1)
                    },
                    |_| ExitCode::SUCCESS,
                ),
                None => {
                    usage();
                    ExitCode::from(2)
                }
            }
        }
        _ => {
            usage();
            ExitCode::from(2)
        }
    }
}
