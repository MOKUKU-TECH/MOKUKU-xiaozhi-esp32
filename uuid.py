uuid = "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
bytes_be = bytes.fromhex(uuid.replace("-", ""))
bytes_le = bytes_be[::-1]
print([f"0x{b:02x}" for b in bytes_le])

str_out = ""
for b in bytes_le:
    str_out = str_out + f"0x{b:02x}, "
print(str_out)
