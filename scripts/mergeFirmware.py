Import("env")
import os

APP_BIN = "$BUILD_DIR/${PROGNAME}.bin"
MERGED_BIN = os.path.join(env['PROJECT_DIR'], "docs", "firmware", "esp32_panel_latest.bin")
BOARD_CONFIG = env.BoardConfig()

def merge_bin(source, target, env):
    print("Merging firmware images")

    # The list contains all extra images (bootloader, partitions, eboot) and
    # the final application binary
    flash_images = env.Flatten(env.get("FLASH_EXTRA_IMAGES", [])) + ["$ESP32_APP_OFFSET", APP_BIN]

    # Run esptool to merge images into a single binary
    env.Execute(
        " ".join(
            [
                "$PYTHONEXE",
                "$OBJCOPY",
                "--chip",
                BOARD_CONFIG.get("build.mcu", "esp32"),
                "merge_bin",
                "--fill-flash-size",
                BOARD_CONFIG.get("upload.flash_size", "4MB"),
                "--flash_mode dio",
                "--flash_freq 40m",
                "-o",
                MERGED_BIN,
            ]
            + flash_images
        )
    )

# Add a post action that runs esptoolpy to merge available flash images
env.AddPostAction("buildprog", merge_bin)
env.AddPostAction("upload", merge_bin)
print("Post build action added for merging firmware")

