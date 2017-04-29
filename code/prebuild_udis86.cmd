@echo off
pushd vendor\udis86
if not exist libudis86\itab.c ( python scripts/ud_itab.py docs/x86/optable.xml ./libudis86/ )
popd
