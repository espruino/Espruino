# Save and Load programs
When we have entered a program, we need to be able to save that program so that when the
device is restarted, the program loads and runs.

The primary mechanism for this is the routine called `jsfSaveToFlash` found in `jswrap_flash.c`.

The signature for this is:

```
void jsfSaveToFlash(JsvSaveFlashFlags flags, JsVar *bootCode)
``` 

It assumes that the program will be saved starting at `FLASH_SAVED_CODE_START`.  The first
word (4bytes) is the amount of boot code saved.  The second word is the end address of
decompressed JS code.  The boot code is saved at FLASH_SAVED_CODE_START+8.  The saved
state starts at FLASH_SAVED_CODE_START + 8 + boot_code_length.

In the following table, the offsets are relative to FLASH_SAVED_CODE_START

| Name                   | Address offset
+------------------------+-----------------------
| boot_code_length       | 0
| <end>                  | 4
| Boot code              | 8
| Saved state            | 8 + boot_code_length

`FLASH_MAGIC` is `0xDEADBEEF`.
`FLASH_MAGIC_LOCATION` is `FLASH_SAVED_CODE_START + FLASH_CODE_LENGTH - 4`.
`FLASH_CODE_LENGTH` is `65536`.

The `ESP32.py` file describes the flasg details in a section called `saved_code`:

```
'saved_code' : {
  'address' : 0x100000,
  'page_size' : 4096,
  'pages' : 16,
  'flash_available' : 960, # firmware can be up to this size
}
```

```
jsfSaveToFlash_writecb(unsigned char ch, uint32_t *cbdata)
```
Write 1 byte specified by `ch` to flash.

```
jsfSaveToFlash_checkcb(unsigned char ch, uint32_t *cbdata)
```
Validate that the character supplied can be read back.

```
bool jsfFlashContainsCode()
```
Validate that the data contains code.