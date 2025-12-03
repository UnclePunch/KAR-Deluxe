import os
import shutil
import sys
import subprocess
from argparse import ArgumentParser
from pyisotools.iso import GamecubeISO
from pathlib import Path

def main(argv):
    if argv is None:
        argv = sys.argv[1:]

    parser = ArgumentParser(
        f"iso.py", description="Creates a modified iso from a base iso and root folder containing additional/modified files.", allow_abbrev=False)
    parser.add_argument("src", help="path to vanilla iso")
    parser.add_argument("replace", help="path to folder containing files to replace in the iso")
    parser.add_argument("dest", help="path to outputted .xdelta file")
    args = parser.parse_args(args=argv)

    src_path  = Path(args.src).resolve()
    out_path = Path(args.dest).resolve()
    replacement_files_path = Path(args.replace).resolve()
    root_folder_path = Path(".temp").resolve()
    iso_path = Path("out.iso").resolve()

    extract_iso(src_path, root_folder_path)
    copy_all_files(replacement_files_path, root_folder_path / "root")
    build_iso(root_folder_path / "root", iso_path)
    remove(root_folder_path)
    create_xdelta_patch(src_path, iso_path, out_path)

    # remove(iso_path)

def extract_iso(iso_path, root_folder):
    print(f"Extracting iso...")
    iso = GamecubeISO.from_iso(iso_path)
    iso.extract(root_folder, dumpPositions=False)
    
def build_iso(root_folder, iso_path):
    print(f"Rebuilding iso...")
    iso = GamecubeISO.from_root(root_folder, genNewInfo=False)
    iso.build(iso_path)

def copy_all_files(src_folder, dst_folder):
    src_path = Path(src_folder)
    dst_path = Path(dst_folder)
    dst_path.mkdir(parents=True, exist_ok=True)

    print(f"Copying files...")
    for file in src_path.rglob("*"):  # Recursively go through everything
        if file.is_file():
            rel_path = file.relative_to(src_path)
            target = dst_path / rel_path
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(file, target)

def create_xdelta_patch(original_iso, modified_iso, patch_file):
    print(f"Creating patch...")
    original_iso = Path(original_iso).resolve()
    modified_iso = Path(modified_iso).resolve()
    patch_file = Path(patch_file).resolve()

    result = subprocess.run([
        "xdelta", "-B", "1363148800", "-e", "-f", "-s",
        str(original_iso),
        str(modified_iso),
        str(patch_file)
    ], capture_output=True, text=True)

    if result.returncode != 0:
        print("Error creating patch:", result.stderr)
    else:
        print("Patch created:", patch_file)

def remove(path):
    if os.path.isfile(path):
        os.remove(path)
    elif os.path.isdir(path):
        shutil.rmtree(path)

if __name__ == "__main__":
    main(sys.argv[1:])