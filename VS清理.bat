@echo Off

del /s /a *.ncb *.user *.pdb *.netmodule *.aps *.ilk *.obj *.sbr *.bsc *.pch *.ipch *.res *.sbr *.idb 2>nul

@REM ����ɾ������ָ����Ŀ¼��
FOR /R . %%d IN (.) DO rd /s /q "%%d\intermediate" 2>nul
FOR /R . %%d IN (.) DO rd /s /q "%%d\log" 2>nul