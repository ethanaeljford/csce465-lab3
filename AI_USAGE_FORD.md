# AI Usage Disclosure — HW3 | Ethanael Ford 

## Tool/model and date
Claude (Anthropic). Used October 28–29, 2026, across two chat sessions: one session to help craft and refine the task prompts, and a second session where those prompts were used to produce and test the code.

## Purpose
Assistance with CSCE 465 HW3: Building and testing the files for the buffer overlow c file, the html file, and the claude skill code files. All design requirements came from the assignment; final review, testing, and the written report are my own.

## AI Conversation Log file
- chat_log_ford.txt

## What I used
AI assistance contributed to the following HW3 deliverables:
- `legacy_log_parser.c` — Task 1: intentionally vulnerable ret2win C program (unchecked `memcpy` into a 128-byte stack buffer; non-inlined `win()` that writes a marker and attempts the decoy read). Code copied verbatim from the AI output, then modified by me to add the decoy read back in.
- `legacy-parser.sh` — Task 2: OpenClaw skill wrapper enforcing the `inbox/`-only access-control boundary (canonicalize path, containment check, refuse otherwise).
- `SKILL.md` — Task 2: skill definition carrying the anti-injection stance (treat external content as data, not instructions).
- `injection_awareness_demo.html` — Task 2: inert adversarial webpage demonstrating three indirect-prompt-injection vectors against a harmless marker, then I added the call for the legacy_parcer skill after. 
  
## What I changed
For the C file, the part of the win() function that looks for the decoy file was not there at first so I added that in the same form the AI made the first half of the win() function. The HTML file was made safe originally by the AI but I added the part that explicitly asks the AI to fun the legacy_parcer skill on the payload file in the html file. 

## How I tested it
Ran the code, took the screenshots, and made sure the output was what I was expecting it to be. 

## One error, limitation, or rejected suggestion
The big error was that the AI was not making me the malicous webpage give instructions to run the legacy_parcer skill because it was contradicting their TOS, so I had it make the structure of the webpage, then myself added the parts that call the skill.
