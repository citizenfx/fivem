@echo off
for /D %%G in (*) do (
    cd %%G
    git pull origin master
    git checkout boost-1.85.0
    cd ..
)