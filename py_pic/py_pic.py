# 把文件内容以byte字节形式读写到缓冲区中。
from PIL import Image
import os


"""
im = Image.open("images/woof.bmp", "r")
print(im.size, im.format, im.mode)
"""

# filename = 'images/test1.txt'
filename = "images/woof.bmp"
# filename = "images/red.bmp"


def read_all_head(filename, head_len):
    buf = bytearray(os.path.getsize(filename))
    with open(filename, 'rb') as f:
        buf = f.read(head_len)
    f.close()
    return buf


def read_bytes(filename, offbits):
    buf = bytearray(64)
    with open(filename, 'rb') as f:
        f.seek(offbits)
        buf = f.read(64)
    f.close()
    return buf


def print_hex_table(bytes):
    for i in range(len(bytes)):
        if i % 16 == 0:
            print("")
            print(str(hex(int(i/16) * 16)) + "\t|", end=" ")
        print(hex(int(bytes[i])), end='\t')
    print("")


def show_image_msg(filename):
    print(filename)
    f = open(filename, "rb")

    # bmp file header
    # 文件标识符，必须为"BM"，即0x424D 才是Windows位图文件
    bfType = bytearray(2)
    bfType = f.read(2)

    # 整个BMP文件的大小（以位B为单位）
    f.seek(0x2)
    bfSize = bytearray(4)
    bfSize = f.read(4)

    # 说明从文件头0000h开始到图像像素数据的字节偏移量（以字节Bytes为单位）
    f.seek(0xa)
    bfOffBits = bytearray(4)
    bfOffBits = f.read(4)

    print("bfType:" + str(bfType, "utf-8"))
    print("bfSize:" + str(int.from_bytes(bfSize, "little")))
    print("bfOffBits:" + str(int.from_bytes(bfOffBits, "little")))

    # bitmap-information header
    # BNP信息头即BMP_INFOHEADER结构体所需要的字节数（以字节为单位）
    f.seek(0xe)
    biSize = bytearray(4)
    biSize = f.read(4)

    # 说明图像的宽度（以像素为单位）
    f.seek(0x12)
    biWidth = bytearray(4)
    biWidth = f.read(4)

    # 说明图像的高度（以像素为单位）
    f.seek(0x16)
    biHeight = bytearray(4)
    biHeight = f.read(4)

    # 为目标设备说明位面数，其值总设置为1
    f.seek(0x1a)
    biPlanes = bytearray(2)
    biPlanes = f.read(2)

    # 说明一个像素点占几位（以比特位/像素位单位）
    f.seek(0x1c)
    biBitCount = bytearray(2)
    biBitCount = f.read(2)

    # 说明图像数据的压缩类型
    f.seek(0x1e)
    biCompression = bytearray(4)
    biCompression = f.read(4)

    # 说明图像的大小，以字节为单位。
    f.seek(0x22)
    biSizeImage = bytearray(4)
    biSizeImage = f.read(4)

    # 说明水平分辨率，用像素/米表示，有符号整数
    f.seek(0x26)
    biXPelsPerMeter = bytearray(4)
    biXPelsPerMeter = f.read(4)

    # 说明垂直分辨率，用像素/米表示，有符号整数
    f.seek(0x2a)
    biYPelsPerMeter = bytearray(4)
    biYPelsPerMeter = f.read(4)

    # 说明位图实际使用的调色板索引数，0：使用所有的调色板索引
    f.seek(0x2e)
    biClrUsed = bytearray(4)
    biClrUsed = f.read(4)

    # 说明对图像显示有重要影响的颜色索引的数目，如果是0，表示都重要
    f.seek(0x32)
    biClrImportant = bytearray(4)
    biClrImportant = f.read(4)

    print("biSize:" + str(int.from_bytes(biSize, "little")))
    print("biWidth:" + str(int.from_bytes(biWidth, "little")))
    print("biHeight:" + str(int.from_bytes(biHeight, "little")))
    print("biPlanes:" + str(int.from_bytes(biPlanes, "little")))
    print("biBitCount:" + str(int.from_bytes(biBitCount, "little")))
    print("biCompression:" + str(int.from_bytes(biCompression, "little")))
    print("biSizeImage:" + str(int.from_bytes(biSizeImage, "little")))
    print("biXPelsPerMeter:" + str(int.from_bytes(biXPelsPerMeter, "little")))
    print("biYPelsPerMeter:" + str(int.from_bytes(biYPelsPerMeter, "little")))
    print("biClrUsed:" + str(int.from_bytes(biClrUsed, "little")))

    f.close()

    return int.from_bytes(bfOffBits, "little")


# print_hex_table(read_all_head(filename, 1280))
# show_image_msg(filename)
# print(show_image_msg(filename))
print_hex_table(read_bytes("images/woof.bmp", show_image_msg("images/woof.bmp")))
print_hex_table(read_bytes("images/red.bmp", show_image_msg("images/red.bmp")))