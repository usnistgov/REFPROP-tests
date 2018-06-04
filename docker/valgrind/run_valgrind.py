import subprocess, os, sys, shutil, glob, timeit

tic = timeit.default_timer()
output = subprocess.run('/REFPROP-tests/build/main --list-tests', shell = True, stdout = subprocess.PIPE).stdout.decode('utf-8')

lines = output.split('\n')[1::]

for il in range(0,len(lines),2):
    line = lines[il]
    tags = lines[il+1]
    if ' test cases' in line: continue
    if 'slow' in tags: continue # Don't run slow tests
    root =  line.strip().lower().replace(' ','_') + '.txt'
    print(tag, ' --> ', root)

    cmd = 'valgrind --tool=memcheck --error-limit=no --track-origins=yes /REFPROP-tests/build/main "' + tag + '"'
    with open('log_'+root,'w') as fp_stderr:
        with open('err_'+root,'w') as fp_stdout:
            subprocess.run(cmd, shell = True, stdout = fp_stdout, stderr = fp_stderr)
    print(open('log_'+root).readlines()[-1])

# Store all the outputs in zip archive
os.makedirs('errlog')
for g in glob.glob('log_*.txt') + glob.glob('err_*.txt'):
    shutil.copy2(g,'errlog')
shutil.make_archive('/output/output','zip','errlog')

toc = timeit.default_timer()
print('elapsed time [s]:', toc-tic)