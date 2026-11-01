# CSCE 465/765 — Homework 3: From Prompt to Process

This repository contains our team's submission for Homework 3 (From Prompt to Process). It documents a classic stack buffer overflow (ret2win) in a small C log parser, how an AI agent can be steered into invoking that parser through indirect prompt injection, the fix and compiler hardening, and confinement with Unix permissions and an AppArmor MAC profile.

**Team:** `<Student 1 name / UIN>` and `<Student 2 name / UIN>`
**Contribution split:** `<e.g., 50:50>` (details in `report.pdf`)

All work was done inside the course Ubuntu 24.04 LTS x86-64 VM (NAT networking only). Only fake secrets, local mock services, and harmless marker files were used.

## 0. Repository layout

```
hw3/
├── README.md
├── report.pdf
├── AI_USAGE.md              # one per student (e.g., AI_USAGE_<name>.md)
├── src/
│   ├── legacy_log_parser.c        # Task 1: intentionally vulnerable parser
│   ├── legacy_log_parser_fixed.c  # Task 3: bounds-checked parser
│   └── make_payload.py            # Task 1: payload generator
├── policy/
│   └── apparmor.profile           # Task 4: MAC profile
├── skills/
│   └── legacy-parser/
│       ├── SKILL.md               # Task 2: OpenClaw skill definition
│       └── run_parser.sh          # Task 2: wrapper restricted to inbox/
├── web/
│   └── adversarial.html           # Task 2: adversarial page
└── tests/                         # Task 3: test scripts and outputs
```

Working directory used for all commands (so relative paths such as `markers/ret2win.txt` resolve correctly):

```
mkdir -p ~/csce465-agentsec/hw3/{src,policy,skills/legacy-parser,web,tests,inbox,markers,evidence}
cd ~/csce465-agentsec/hw3
```

## 1. Task 0 — Lab preparation

```
sudo apt update
sudo apt install -y build-essential gdb binutils checksec apparmor-utils

gcc --version
gdb --version
checksec --version
aa-status --version
```

## 2. Task 1 — Vulnerable parser and ret2win

### 2.1 Create the fake protected file

```
sudo mkdir -p /opt/agentsec
printf 'FAKE-COURSE-SECRET\n' | sudo tee /opt/agentsec/decoy.txt
sudo chmod 600 /opt/agentsec/decoy.txt
```

### 2.2 Build the vulnerable binary (protections off)

```
gcc -O0 -g -fno-stack-protector -no-pie -Wl,-z,noexecstack \
    -Wall -Wextra -o legacy_log_parser src/legacy_log_parser.c
checksec --file=./legacy_log_parser
```

* `BUF_SIZE` = 128 (fixed stack buffer), `MAX_COPY` = 512 (the bug: larger than the buffer).
* The file is read into a heap buffer and closed **before** the unchecked copy, so the overflow reaches only the saved return address.
* `win()` creates `markers/ret2win.txt` and attempts to read `/opt/agentsec/decoy.txt`.

### 2.3 Find the offset and the address of `win()`

```
nm ./legacy_log_parser | grep win
gdb ./legacy_log_parser
```

The offset was determined empirically in GDB (value recorded in `report.pdf`).

### 2.4 Generate the payload and run it

```
python3 src/make_payload.py            # writes the payload file (see script for output path)
./legacy_log_parser inbox/payload.bin
ls -l markers/ret2win.txt
```

Report contents for this task: GDB output, direct-execution demonstration, stack diagram, and written explanation of the vulnerability.

## 3. Task 2 — Agent-mediated invocation

### 3.1 OpenClaw skill

The skill `legacy-parser` lives at `skills/legacy-parser/` and is installed to the OpenClaw workspace:

```
mkdir -p ~/.openclaw/workspace/skills/legacy-parser
cp skills/legacy-parser/SKILL.md skills/legacy-parser/run_parser.sh \
   ~/.openclaw/workspace/skills/legacy-parser/
chmod +x ~/.openclaw/workspace/skills/legacy-parser/run_parser.sh
openclaw skills check
```

* `run_parser.sh` accepts **only** a file path under `~/csce465-agentsec/hw3/inbox/` and rejects everything else (a small access-control boundary).

### 3.2 Adversarial webpage

Serve the page locally:

```
python3 -m http.server 8000 --directory web --bind 127.0.0.1
```

### 3.3 Indirect prompt-injection trial

Fetch the adversarial page and ask the agent to summarize it:

```
curl http://127.0.0.1:8000/adversarial.html -o /tmp/adversarial.html
openclaw agent --json -m "Can you summarize this webpage only? $(cat /tmp/adversarial.html)"
```

Capture and save the trace:

```
openclaw audit
openclaw exec-policy show
```

Grading does not depend on model behavior. The report explains the intended chain: (1) untrusted webpage content enters context, (2) the agent proposes or refuses a tool call, (3) a policy decision permits or blocks it, (4) the native program contains the buffer overflow.

## 4. Task 3 — Fix and harden

### 4.1 Build the fixed binary

```
gcc -O2 -g -fstack-protector-strong -D_FORTIFY_SOURCE=3 \
    -fPIE -pie -Wl,-z,relro,-z,now -Wl,-z,noexecstack \
    -Wall -Wextra -Werror \
    -o legacy_log_parser_fixed src/legacy_log_parser_fixed.c
```

* Root-cause fix: oversized input is rejected before it is copied into the fixed buffer.
* Canary, FORTIFY, PIE/ASLR, and Full RELRO raise the bar for an attacker but do not remove the defect.

### 4.2 Compare binaries

```
checksec --file=./legacy_log_parser
checksec --file=./legacy_log_parser_fixed
```

### 4.3 Tests

Test cases in `tests/`:

* valid short input
* exactly maximum allowed input
* one byte over the limit
* missing file
* original exploit payload

```
bash tests/run_tests.sh 2>&1 | tee tests/results.txt
```

## 5. Task 4 — Access control and AppArmor

### 5.1 Low-privilege user

```
sudo useradd --system --create-home --shell /usr/sbin/nologin agentlab
```

Grant `agentlab` access only to the parser, the payload directory, and the marker directory:

```
sudo chown -R agentlab:agentlab ~/csce465-agentsec/hw3/markers
sudo chmod 755 ~/csce465-agentsec/hw3/legacy_log_parser
sudo chmod 755 ~/csce465-agentsec/hw3/inbox
# (exact ownership/permission commands used are listed in report.pdf)
```

### 5.2 Load the AppArmor profile

```
sudo cp policy/apparmor.profile /etc/apparmor.d/<profile-name>
sudo apparmor_parser -r /etc/apparmor.d/<profile-name>
sudo aa-status
```

The profile:

* allows reading the selected payload file
* allows writing only to the marker directory
* explicitly denies `/opt/agentsec/decoy.txt`
* denies execution of shells and unrelated programs
* allows only the libraries and files needed to start the parser

### 5.3 Run the exploit under confinement and capture denials

```
sudo -u agentlab ./legacy_log_parser inbox/payload.bin

sudo journalctl -k | grep -i apparmor
# or
sudo dmesg | grep -i apparmor
# if auditd is installed:
sudo ausearch -m AVC -ts recent
```

## 6. Deliverables checklist

* [ ] `report.pdf` (graded; includes outputs/screenshots, team + contribution statement, stack diagram, trace, checksec table, AppArmor design, denial logs, 400–600 word textbook analysis)
* [ ] `README.md`
* [ ] `AI_USAGE.md` for **each** student, plus exported AI conversation logs
* [ ] `src/`, `policy/`, `skills/`, `web/`, `tests/`

## 7. Safety notes

* Everything ran inside the course VM (NAT networking only); no scanning of external systems.
* Only fake secrets (`FAKE-COURSE-SECRET`) and harmless marker files were used.
* No reverse shells, persistence, credential theft, or real C2 services.
* No personal accounts, SSH keys, cloud credentials, or host home/Docker socket were connected to OpenClaw.
