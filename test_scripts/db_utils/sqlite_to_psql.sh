#!/bin/bash

echo "Dumping database..."
sqlite3 "$1" .dump > "$1".sql
echo "Removing pragmas..."
sed '/^PRAGMA/d' "$1".sql> "$1".clean.0
echo "Removing begin..."
sed '/^BEGIN/d' "$1".clean.0 > "$1".clean.1
echo "Blobs with bytea..."
sed 's/BLOB/BYTEA/g' "$1".clean.1 > "$1".clean.0
echo "Fixing binary fields..."
sed 's/,X/,/g' "$1".clean.0 > "$1".clean.1
echo "Fixing line feed ..."
sed 's/char(10)/chr(10)/g' "$1".clean.1 > "$1".clean.0
echo "Removing commits..."
sed '/^COMMIT/d' "$1".clean.0 > "$1".psql
rm "$1".clean.0
rm "$1".clean.1
rm "$1".sql
