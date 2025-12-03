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
        f"dol.py", description="Extracts .dol file from iso to a specified location.", allow_abbrev=False)
    parser.add_argument("iso", help="path to vanilla iso")
    parser.add_argument("dol", help="path to outputted .dol file")
    args = parser.parse_args(args=argv)

    iso_path  = Path(args.iso).resolve()
    dol_path = Path(args.dol).resolve()

    iso = GamecubeISO.from_iso(iso_path)

    dol_path.parent.mkdir(parents=True, exist_ok=True)
    with open(dol_path, "wb") as f:
        iso.dol.save(f)

if __name__ == "__main__":
    main(sys.argv[1:])