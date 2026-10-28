"""Keep ESP32 BLE Mouse 0.3.1 compatible with Arduino-ESP32 2.x and 3.x."""

from pathlib import Path

Import("env")


def patch_ble_mouse():
    source_path = (
        Path(env.subst("$PROJECT_LIBDEPS_DIR"))
        / env.subst("$PIOENV")
        / "ESP32 BLE Mouse"
        / "BleMouse.cpp"
    )
    source = source_path.read_bytes()
    patched = source

    # const char* converts to std::string (core 2.x) or String (core 3.x).
    for original, replacement in (
        (
            b"BLEDevice::init(bleMouseInstance->deviceName);",
            b"BLEDevice::init(bleMouseInstance->deviceName.c_str());",
        ),
        (
            b"manufacturer()->setValue(bleMouseInstance->deviceManufacturer);",
            b"manufacturer()->setValue(bleMouseInstance->deviceManufacturer.c_str());",
        ),
    ):
        if patched.count(original) == 1:
            patched = patched.replace(original, replacement, 1)
        elif patched.count(replacement) != 1:
            raise RuntimeError(
                f"Unexpected ESP32 BLE Mouse source in {source_path}; "
                "review the Arduino string compatibility patch."
            )

    if patched != source:
        source_path.write_bytes(patched)
        print("Patched ESP32 BLE Mouse string compatibility")

    connection_path = source_path.with_name("BleConnectionStatus.cpp")
    source = connection_path.read_bytes()
    patched = source.replace(
        b"  desc->setNotifications(",
        b"  if (desc != nullptr) desc->setNotifications(",
    )
    if patched != source:
        connection_path.write_bytes(patched)


if not env.IsCleanTarget():
    patch_ble_mouse()
