import sys
import struct

PLAYER_DESC_SIZE = 0x30
MISC_SIZE = 0xac4 - 0xa94

FRAME_PLAYER_COUNT = 4
STADIUM_PLAYER_COUNT = 5

def read(fmt, f):
    size = struct.calcsize(fmt)
    data = f.read(size)
    if len(data) != size:
        raise EOFError("Unexpected EOF")
    return struct.unpack(fmt, data)

def dump_starpole(input_path, output_path):
    with open(input_path, "rb") as f, open(output_path, "w") as out:

        # -----------------------------
        # StarpoleDataMatch
        # -----------------------------

        rng_seed, = read(">I", f)
        frame_size, = read(">H", f)
        stage_kind, = read(">H", f)
        stadium_kind, = read(">B", f)

        misc = f.read(MISC_SIZE)

        out.write("=== StarpoleDataMatch ===\n")
        out.write(f"rng_seed      : {rng_seed:08X}\n")
        out.write(f"frame_size    : {frame_size}\n")
        out.write(f"stage_kind    : {stage_kind}\n")
        out.write(f"stadium_kind  : {stadium_kind}\n")
        out.write(f"misc[{len(misc)}]\n\n")

        # stadium.ply_stats[5][9]
        ply_stats = []
        out.write("stadium.ply_stats:\n")
        for p in range(STADIUM_PLAYER_COUNT):
            stats = []
            for s in range(9):
                val, = read(">b", f)
                stats.append(val)
            ply_stats.append(stats)
            out.write(f"  player {p}: {stats}\n")

        # stadium.is_bike[5]
        is_bike = []
        out.write("\nstadium.is_bike:\n")
        for p in range(STADIUM_PLAYER_COUNT):
            val, = read(">B", f)
            is_bike.append(val)
            out.write(f"  player {p}: {val}\n")

        # stadium.machine_kind[5]
        machine_kind = []
        out.write("\nstadium.machine_kind:\n")
        for p in range(STADIUM_PLAYER_COUNT):
            val, = read(">B", f)
            machine_kind.append(val)
            out.write(f"  player {p}: {val}\n")

        # PlayerDesc ply_desc[4]
        out.write("\nPlayerDesc[4]:\n")
        for i in range(4):
            raw = f.read(PLAYER_DESC_SIZE)
            out.write(f"  ply_desc[{i}]: {raw.hex()}\n")

        out.write("\n")

        # -----------------------------
        # StarpoleDataFrame array
        # -----------------------------

        out.write("=== StarpoleDataFrame ===\n")

        for i in range(4360):
            out.write(f"\n-- Frame {i} --\n")

            frame_idx, = read(">I", f)
            rng_seed, = read(">I", f)
            ply_num, = read(">B", f)

            out.write(f"frame_idx : {frame_idx}\n")
            out.write(f"rng_seed  : {rng_seed:08X}\n")
            out.write(f"ply_num   : {ply_num}\n")

            for p in range((frame_size - 9) // 8):
                idx, = read(">B", f)
                held, stickX, stickY, subX, subY, trigger = read(">HbbbbB", f)

                if (p < ply_num):
                    out.write(
                        f"  ply[{p}]: "
                        f"idx={idx} "
                        f"held=0x{held:04X} "
                        f"stick=({stickX},{stickY}) "
                        f"sub=({subX},{subY}) "
                        f"trig={trigger}\n"
                    )

        # Check for trailing data
        remaining = f.read()
        if remaining:
            out.write(
                f"\nWARNING: {len(remaining)} trailing bytes at EOF\n"
            )

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("usage: python dump_starpole.py <input.bin> <output.txt>")
        sys.exit(1)

    dump_starpole(sys.argv[1], sys.argv[2])
