# GBCC
GameBoy Color emulator written in C.

Currently state:
  - Almost all test roms from Blargg & Mooneye pass.
  - All games tested so far do something, though a fair few are buggy.
  - Speed is good, though only under Clang. GCC produces a binary about half as
    fast (probably from not simplifying the various gbcc_memory_read functions
    as much)
  - Most accurate colour emulation available (at least to my eye).
  - Savestates & cartridge saves are working fine.
