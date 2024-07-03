@echo Off

del /s /a *.ncb *.user *.pdb *.netmodule *.aps *.ilk *.obj *.sbr *.bsc *.pch *.ipch *.res *.sbr *.idb 2>nul

@REM 遍历删除所有指定的目录名
FOR /R . %%d IN (.) DO rd /s /q "%%d\intermediate" 2>nul
FOR /R . %%d IN (.) DO rd /s /q "%%d\log" 2>nul