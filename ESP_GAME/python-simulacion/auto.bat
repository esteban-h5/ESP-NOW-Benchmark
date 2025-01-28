@echo off

for /L %%e in (1,1,3) do (
    for /L %%p in (0,10,100) do (
        python main.py -eA %%e -eB %%e -pA %%p
    )
)
