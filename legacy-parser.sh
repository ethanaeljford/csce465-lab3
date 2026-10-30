#!/usr/bin/env bash
#
# legacy-parser — OpenClaw skill wrapper
#
# Purpose: run the compiled ~/csce465-agentsec/hw3/legacy_log_parser on a
# single file, but ONLY if that file truly lives inside the allowed inbox
# directory. The canonicalize-then-check logic below is the access-control
# boundary: it stops the agent from being talked into running the parser on
# arbitrary files via symlinks or "../" traversal.
#
# Usage: legacy-parser <file-path-inside-inbox>

set -euo pipefail

# ---- fixed locations (the boundary is hard-coded on purpose) --------------
readonly HW3_DIR="$HOME/csce465-agentsec/hw3"
readonly INBOX="$HW3_DIR/inbox"
readonly PARSER="$HW3_DIR/legacy_log_parser"

die() {
    # Print a clear error and exit non-zero WITHOUT running the parser.
    printf 'legacy-parser: %s\n' "$1" >&2
    exit 1
}

# ---- 1. argument check ----------------------------------------------------
if [[ $# -ne 1 ]]; then
    die "usage: legacy-parser <file-path-inside-inbox>"
fi
raw_path="$1"

# ---- 2. canonicalize the boundary itself ----------------------------------
# Resolve symlinks/".." in the inbox path so the later comparison is
# apples-to-apples. It must exist and be a directory.
if ! real_inbox="$(realpath --canonicalize-existing -- "$INBOX" 2>/dev/null)"; then
    die "inbox directory does not exist: $INBOX"
fi
[[ -d "$real_inbox" ]] || die "inbox path is not a directory: $real_inbox"

# ---- 3. canonicalize the user-supplied path -------------------------------
# --canonicalize-existing resolves EVERY symlink and "." / ".." segment and
# requires the final target to actually exist. A path that doesn't resolve
# (dangling symlink, nonexistent file) is refused here.
if ! real_path="$(realpath --canonicalize-existing -- "$raw_path" 2>/dev/null)"; then
    die "path does not exist or cannot be resolved: $raw_path"
fi

# ---- 4. containment check (the security boundary) -------------------------
# The resolved real path must live strictly inside the resolved inbox.
# We compare "$real_path/" against the pattern "$real_inbox"/* so that a
# sibling directory such as ".../hw3/inbox-evil/x" can NOT masquerade as
# being inside ".../hw3/inbox". (Matching on the bare prefix would be the
# classic bug; the trailing slash is what makes it correct.)
case "$real_path/" in
    "$real_inbox"/*) : ;;  # inside the inbox — allowed
    *) die "REFUSED: '$real_path' is outside the allowed inbox ($real_inbox)" ;;
esac

# Only a regular file is acceptable (rejects the inbox dir itself, devices,
# FIFOs, etc.).
[[ -f "$real_path" ]] || die "REFUSED: not a regular file: $real_path"

# ---- 5. run the parser ----------------------------------------------------
[[ -x "$PARSER" ]] || die "parser binary not found or not executable: $PARSER"

# Run from hw3/ so the parser's relative marker path (markers/ret2win.txt)
# resolves where the lab expects. exec hands stdout/stderr straight through,
# so the parser's output is printed verbatim.
cd "$HW3_DIR" || die "cannot cd to $HW3_DIR"
exec "$PARSER" "$real_path"
