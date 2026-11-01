# AI Usage Description

This document outlines how AI tools were used to assist with this assignment.

## Summary

AI was used in this homework primarily to clear up gaps in my knowledge of stack buffer overflows, compiler/linker hardening flags, and AppArmor's profile syntax, and to help debug build and shell errors as I worked through the VM. I did not use AI to write any part of the report as all report content reflects my own understanding.

## Detailed Breakdown

### Report
- **No AI use.** The report was written entirely by me.

### Markdown Files
- I wrote most of the content in the README myself, following the HW pdf's structure for environment setup and commands.
- Used AI to help with `.md` file **syntax and formatting** to make the files look polished.

### legacy_log_parser_fixed.c (Task 3)
- I understood the vulnerability and the fix (the missing bounds check) myself from Task 1, but I had AI help write out the full corrected source so I didn't have to retype the unchanged parts (`parse_log`, `main`, etc.) by hand, and I swapped in my own `win()` function from my Task 1 code so the fixed binary's behavior matches the vulnerable one exactly.
- I had AI explain what each hardening flag (`-fstack-protector-strong`, `-D_FORTIFY_SOURCE=3`, `-fPIE -pie`, `-Wl,-z,relro,-z,now`) actually does, since the assignment names them but doesn't explain them in depth, and I used that understanding to write the report explanation in my own words.

### run_tests.sh (Task 3)
- I had AI help generate the test script structure covering the five required cases (valid input, max input, over-limit input, missing file, exploit payload), but I fixed the directory paths myself once I found the script assumed the wrong layout for my VM.
- I ran the tests myself and confirmed the actual pass/fail output before including it in the report.

### make_payload.py
- I had AI help write this script since I wasn't sure how to reproducibly package the offset and `win()` address into a Python script. I verified it myself by re-deriving the offset in GDB and the address with `nm`, then diffing the script's output against my known-working payload to confirm it matched before trusting it.

### policy/apparmor.profile (Task 4)
- I was unfamiliar with AppArmor profile syntax, so I had AI explain the rule format (`r`, `rw`, `audit deny`, `#include <abstractions/base>`) and draft an initial profile mapped to the five stated requirements.
- I adjusted the file paths myself to match my actual `/srv/agentlab/hw3` directory layout, loaded the profile myself, and verified it was running in enforce mode and producing real denial logs before including anything in the report.

### Debugging
- I had AI help debug a few shell issues I ran into directly: a `gcc` link error from compiling an empty source file, a missing `payload.bin` after reorganizing my directories (which I regenerated and re-verified myself), and an `aa-complain: command not found` error that turned out to be a PATH issue rather than a missing package.

## Tools Used

- Claude
- Course assignment PDF (for command references)
