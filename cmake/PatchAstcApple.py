import sys

MARKER = "Patched by sc2fla"

OLD = '''elseif(APPLE)
    # On Apple platforms, we build a universal binary for both x86_64 and arm64
    add_custom_target(
        astcenc ALL
        COMMAND
        lipo -create -output $<TARGET_FILE_DIR:astcenc-sse4.1-static>/libastcenc-static.a -arch x86_64 $<TARGET_FILE:astcenc-sse4.1-static> -arch x86_64h $<TARGET_FILE:astcenc-avx2-static> -arch arm64 $<TARGET_FILE:astcenc-neon-static>
        VERBATIM

        DEPENDS
        astcenc-sse4.1-static
        astcenc-avx2-static
        astcenc-neon-static
    )'''

NEW = f'''elseif(APPLE)
    # {MARKER}
    set(ASTCENC_ISA_NATIVE ON)

    set(ASTC_PREFIX "native")

    add_custom_target(astcenc ALL DEPENDS astcenc-native-static)'''


def main():
    if len(sys.argv) != 2:
        print("usage: PatchAstcApple.py <path-to-astc.cmake>", file=sys.stderr)
        return 0

    path = sys.argv[1]
    try:
        with open(path, encoding="utf-8") as f:
            src = f.read()
    except OSError as exc:
        print(f"warning: could not read {path}: {exc} - astc.cmake will NOT be patched", file=sys.stderr)
        return 0

    if MARKER in src:
        return 0

    if OLD not in src:
        print(f"warning: {path} did not contain the expected upstream APPLE branch - leaving it untouched", file=sys.stderr)
        return 0

    src = src.replace(OLD, NEW, 1)
    with open(path, "w", encoding="utf-8") as f:
        f.write(src)
    return 0


if __name__ == "__main__":
    sys.exit(main())
