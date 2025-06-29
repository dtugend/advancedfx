#![windows_subsystem = "windows"]

use std::process::ExitCode;
use clap::Parser;
use std::io::prelude::*;
use std::fs::File;
use fd_lock::RwLock;
use arboard::Clipboard;
use tray_item::{IconSource, TrayItem};
use std::sync::mpsc;
use std::path::Path;
use std::process::Stdio;
use directories::ProjectDirs;
use core::error::Error;
use std::ops::Deref;
use serde::{Serialize,Deserialize};

#[derive(Parser)]
#[command(author, version, about, long_about = None)]
struct Cli {
    /// Start and wait for main enpdoint fork if there's none yet (recommended).
    #[arg(exclusive = true, long, action=clap::ArgAction::SetTrue)]
    auto_create: bool,

    /// This is a main fork (internal).
    #[arg(exclusive = true, long, action=clap::ArgAction::SetTrue)]
    main_fork: bool,

    /// Prints the OpenRPC schema.
    #[arg(exclusive = true, long, default_value_t=false)]
    print_openrpc_schema: bool
}


#[derive(Serialize, Deserialize)]
pub struct AfxCoreEndpoint {
    id: String,
    transport: String,
    meta: String,
    secret: String
}

#[derive(Serialize)]
pub enum AfxCoreResult {
    Ok{ created: bool, endpoint: AfxCoreEndpoint },
    Error{ message: String }
}


enum Message {
    Quit,
    Copy
}

fn attach_console() -> bool {
    #[cfg(target_os = "windows")]
    {
        use winapi::um::wincon::{AttachConsole, ATTACH_PARENT_PROCESS};
        return unsafe {
           0 !=  AttachConsole(ATTACH_PARENT_PROCESS)
        };
    }
}

fn detach_console() {
    #[cfg(target_os = "windows")]
    {
        use winapi::um::wincon::{GetConsoleWindow,FreeConsole};
        use winapi::um::winuser::{SendMessageA,WM_CHAR};
        unsafe {
            let cw = GetConsoleWindow();

            if !cw.is_null() {
                // Send VK_ENTER
                SendMessageA(cw, WM_CHAR, 13, 0);
            }

            FreeConsole();
        }
    }
}

fn check_main(lock_file_path: &Path, main_lock_file_path: &Path) -> Result<bool,Box<dyn Error>> {
    let mut lock = RwLock::new(File::open(lock_file_path)?);
    let _write_lock = lock.write()?;

    let mut main_lock = RwLock::new(File::open(main_lock_file_path)?);
    if let Ok(_main_write_lock) = main_lock.try_write() {
        return Ok(false);
    }

    Ok(true)
}

fn do_non_main(lock_file_path: &Path, main_lock_file_path: &Path) -> Result<AfxCoreEndpoint,Box<dyn Error>> {
    let lock = RwLock::new(File::open(lock_file_path)?);
    let read_lock = lock.read()?;

    let mut main_lock = RwLock::new(File::open(main_lock_file_path)?);
    if let Err(_) = main_lock.try_write() {
        let mut data = String::new();
        read_lock.deref().read_to_string(&mut data)?;
        let e: AfxCoreEndpoint = serde_json::from_str(&data)?;
        return Ok(e);
    }    

    Err("Main process not present".into())
}

fn get_random_u128() -> Result<u128, getrandom::Error> {
    let mut buf = [0u8; 16];
    getrandom::fill(&mut buf)?;
    Ok(u128::from_ne_bytes(buf))
}

fn get_random_secret() -> Result<String,Box<dyn Error>> {
    let rand0 = get_random_u128().or::<Box<dyn Error>>(Err("Random number generation failed.".into()))?;
    let rand1 = get_random_u128().or::<Box<dyn Error>>(Err("Random number generation failed.".into()))?;
    Ok(format!("{:032x}{:032x}", rand0, rand1))
}

fn do_create_main(lock_file_path: &Path, main_lock_file_path: &Path) -> Result<(),Box<dyn Error>> {
    let mut lock = RwLock::new(File::create(lock_file_path)?);
    let mut write_lock = lock.write()?;

    let mut main_lock = RwLock::new(File::create(main_lock_file_path)?);
    if let Ok(_main_write_lock) = main_lock.try_write() {
        let secret = get_random_secret()?;
        let endpoint = AfxCoreEndpoint{
            id: "afx.core".to_string(),
            transport: "ws".to_string(),
            meta: "ws://127.0.0.1:1234".to_string(),
            secret: secret
        };

        write!(write_lock, "{}", serde_json::to_string(&endpoint)?)?;

        drop(write_lock);

        let mut clipboard = Clipboard::new().unwrap();

        let (tx, rx) = mpsc::sync_channel(1);

        let mut tray = TrayItem::new(
            "afx-core",
            IconSource::Resource("afx-core-icon"),
        )
        .unwrap();

        tray.add_label("afx-core - advancedfx.org").unwrap();

        tray.inner_mut().add_separator().unwrap();

        let copy_tx = tx.clone();
        tray.add_menu_item("Copy endpoint secret", move || {
            copy_tx.send(Message::Copy).unwrap();
        })
        .unwrap();

        tray.inner_mut().add_separator().unwrap();

        let quit_tx = tx.clone();
        tray.add_menu_item("Quit", move || {
            quit_tx.send(Message::Quit).unwrap();
        })
        .unwrap();

        loop {
            match rx.recv() {
                Ok(Message::Copy) => {
                    clipboard.set_text("Hello World!").unwrap();
                }           
                Ok(Message::Quit) => {
                    break;
                }           
                _ => {}
            }
        }

        return Ok(());
    }

    Err("main already present".into())
}

fn do_main_fork() -> Result<(),Box<dyn Error>> {
    let current_exe = std::env::current_exe()?;

    std::process::Command::new(current_exe).arg("--main-fork").arg("true")
        .stdin(Stdio::null())
        .stdout(Stdio::null())
        .stderr(Stdio::inherit())
        .spawn()?;

    Ok(())
}

fn main_do(cli: Cli) -> Result<Option<AfxCoreResult>,Box<dyn Error>> {
    let project_dirs = ProjectDirs::from("org", "advancedfx",  "afx-core").ok_or::<Box<dyn Error>>("Could not get project dirs.".into())?;
    let lock_folder = project_dirs.cache_dir();
    std::fs::create_dir_all(lock_folder)?; 
    let main_lock_file_pathbuf = Path::new(lock_folder).with_file_name("afx-core.main.lock");
    let main_lock_file_path = main_lock_file_pathbuf.deref();
    let lock_file_pathbuf = Path::new(lock_folder).with_file_name("afx-core.json.lock");
    let lock_file_path = lock_file_pathbuf.deref();
    if cli.main_fork {
        do_create_main(lock_file_path, main_lock_file_path)?;
    } else {
        let mut created = false;
        if cli.auto_create {
            if !(check_main(lock_file_path, main_lock_file_path).unwrap_or(false)) {
                do_main_fork()?;
                created = true;
                let a_second = std::time::Duration::from_secs(1);
                let mut seconds = 0;
                while {
                    if 600 <= seconds {
                        return Err("Failed to auto create main within 10 minutes.".into());
                    }
                    std::thread::sleep(a_second);
                    seconds += 1;   
                    !(check_main(lock_file_path, main_lock_file_path).unwrap_or(false))
                } {}
            }
        }
        return Ok(Some(AfxCoreResult::Ok{ created: created, endpoint: do_non_main(lock_file_path, main_lock_file_path)? }));
    }

    Ok(None)
}

const OPENRPC_JSON : &str =  include_str!(concat!(env!("OUT_DIR"), "/afx-core-rpc.json"));

fn main() -> ExitCode {
    let mut exit_code = ExitCode::FAILURE;

    let console = attach_console();

    match Cli::try_parse() {
        Ok(cli) => {
            if cli.print_openrpc_schema {
                println!("{}", OPENRPC_JSON);
            } else {     
                let main_fork: bool = cli.main_fork;
                match main_do(cli) {
                    Ok(eo) => {
                        if let Some(e) = eo {
                            let json_result = serde_json::to_string(&e).unwrap();
                            println!("{}", json_result);
                        } else if !main_fork {
                            // Dead code path.
                            let json_result = serde_json::to_string(&AfxCoreResult::Error{message: "Endpoint not found.".to_string()}).unwrap();
                            println!("{}", json_result);
                        }
                        exit_code = ExitCode::SUCCESS;
                    }
                    Err(e) => {
                        if main_fork {
                            eprintln!("{}", e);
                        } else {
                            let json_result = serde_json::to_string(&AfxCoreResult::Error{message: e.to_string()}).unwrap();
                            println!("{}", json_result);
                        }
                    }
                };
            }
        }
        Err(err) => {
            print!("\n{}", err);
        }
    }

    if console {
        detach_console();
    }

    exit_code
}
