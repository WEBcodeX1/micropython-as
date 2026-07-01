# Filesystem Component

The filesystem component provides file-level access for other components, specifically supporting the HTTP/1.1 server's GET subsystem.
Initially, utilizing SPIFFS or LittleFS on a dedicated partition seemed like a viable approach.

## 1. Custom Implementation

However, a custom, streamlined solution combined with the *falcon-as* HTTP/1.1 *httpparser* and *httpgenerator* libraries proved to be significantly faster and more lightweight. This approach eliminates the need for a dedicated partition. File contents are automatically linked into FlashROM **without** requiring custom linker attributes or additional linker scripts.

Furthermore, this layout enables direct access to the existing **static file buffer** (generated at compile time) *without* **copying a single byte**—a feature fully leveraged by the *httpgenerator*. By bypassing dynamic buffer allocation and underlying file read() operations, all files are instantly available upon microcontroller startup.

## 2. Implementation Details

1. Static Const Definition
2. Metadata Structs
3. Python Converter Script

## 2.1. Static Const Definition

Defining file content as `static const unsigned char filename[filesize] = { 0x3c, ... };` in `filedata.h` instructs the linker to place the segment data into FlashROM instead of ProgramROM. This approach enables the storage of numerous or large files.

## 2.2. Metadata Structs

The `filemetadata.h` header file contains the references (URL+filename, file type, file start pointer, and file size) for each file. It also includes a compile-time generated `std::array` to query file metadata at runtime.

## 2.3. Python Converter Script

File metadata is defined in `filemetadata.py`. Running the included Python 3 script (`python3 convert_static_fs.py`) converts these files into `filedata.h` and `filemetadata.h`. Once generated, you can proceed with the standard compilation process (`idf.py build`).

## 3. Future Outlook

For use cases where maximum performance is not a strict requirement, integrating a *LittleFS* or *SPIFFS* layer remains a possibility. This implementation may be considered for future updates, though it is not currently planned for the near term.
