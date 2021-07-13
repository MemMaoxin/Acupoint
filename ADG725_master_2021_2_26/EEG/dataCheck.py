import numpy

if __name__ == "__main__":
    data_bytes = [0xFF, 0xFF, 0xFF, 0xFF]
    #[0x80, 0X00, 0X00, 0X00]
    k2 = 0
    data_put = data_bytes[4 * k2] * 16777216 + data_bytes[1 + 4 * k2] * 65536 + data_bytes[2 + 4 * k2] * 256 + data_bytes[3 + 4 * k2]
    if data_put > 2147483647:
        data_put = data_put - 4294967296
    print(data_put)
