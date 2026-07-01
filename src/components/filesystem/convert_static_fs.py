import os
import sys
import filemetadata

def write_filedata(infile, file_props, file_number):

    try:
        inputfh = open(input_filename, "rb")
        filedata = inputfh.read()
    except:
        print("open input file:{} failed".format(infile))
        sys.exit(2)

    inputfh.close()

    # write file data (binary content)
    try:
        outfh = open("filedata.h", "a")
    except:
        print("open output file:{} failed".format(outfh))
        sys.exit(2)

    file_length = len(filedata)
    file_id = "file{}".format(file_number)

    print(file_length)

    outfh.write("static const unsigned char " + file_id + "[" + str(file_length) + "] = {")

    for x in range(len(filedata)):
        if (x % 16 == 0):
            outfh.write("\n")
            outfh.write("\t")
        outfh.write("0x" + hex(filedata[x])[2:].zfill(2) + ", ")

    outfh.write("\n};\n")
    outfh.close()

    # write file metadata (structs)
    try:
        outfh = open("filemetadata.h", "a")
    except:
        print("open output file:{} failed".format(outfh))
        sys.exit(2)

    outfh.write(
        'static const ServerFile f{} = {{ "{}", "{}", {}, {} }};'.format(
            file_number,
            file_props[0],
            file_props[1],
            file_id,
            str(file_length)
        )
    )

    outfh.write("\n")
    outfh.close()


if __name__ == "__main__":

    try:
        os.unlink("filedata.h")
        os.unlink("filemetadata.h")
    except:
        pass

    processfiles = filemetadata.data

    file_number = 0;
    for input_filename, file_props in processfiles.items():
        file_number += 1
        write_filedata(input_filename, file_props, file_number)

    try:
        outfh = open("filemetadata.h", "a")
    except:
        print("open output file:{} failed".format(outfh))
        sys.exit(2)

    filerefs = [ "f{},".format(x) for x in range(1, file_number+1) ]
    print(filerefs)

    outfh.write(
        'static const std::array<ServerFile, {}> ServerFiles = {{ {} }};'.format(
            file_number,
            ' '.join(filerefs)
        )
    )
