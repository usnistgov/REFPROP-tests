import subprocess, os, sys, shutil, glob, timeit

tic = timeit.default_timer()
output = subprocess.run('/REFPROP-tests/build/main -t', shell = True, stdout = subprocess.PIPE).stdout.decode('utf-8')

for il, line in enumerate(output.split('\n')[1::]):

    if not line or '[' not in line: continue
    tag = '[' + line.split('[')[1]
    if 'Torture' in tag: continue # Don't run the Torture test (very slow)
    root =  tag.replace('[', '').replace(']','') + '.txt'
    print(tag, ' --> ', root)
    continue

    cmd = 'valgrind --tool=memcheck --error-limit=no --track-origins=yes /REFPROP-tests/build/main ' + tag
    with open('log_'+root,'w') as fp_stderr:
        with open('err_'+root,'w') as fp_stdout:
            subprocess.run(cmd, shell = True, stdout = fp_stdout, stderr = fp_stderr)

subprocess.run('for f in log_*.txt; do echo $f && tail -n 1 $f; done', shell=True, stdout = sys.stdout, stderr = sys.stderr)

# Store all the outputs in zip archive
os.makedirs('errlog')
for g in glob.glob('log_*.txt') + glob.glob('err_*.txt'):
    shutil.copy2(g,'errlog')
shutil.make_archive('/output/output','zip','errlog')

toc = timeit.default_timer()
print('elapsed time [s]:', toc-tic)