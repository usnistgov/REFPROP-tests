import subprocess, os, sys, shutil, glob, timeit

tic = timeit.default_timer()
output = subprocess.run('/REFPROP-tests/build/main -t', shell = True, stdout = subprocess.PIPE).stdout.decode('utf-8')

# Wipe contents of output folder 
# This script is running inside the docker container, so this should be allowed
for g in glob.glob('/output/*'):
    if os.path.isdir(g):
        shutil.rmtree(g)
    else:
        os.remove(g)

for il, line in enumerate(output.split('\n')[1::]):

    if not line or '[' not in line: continue
    tag = '[' + line.split('[')[1]
    if 'veryslow' in tag: continue # Don't run the veryslow tests
#    if 'predef_mix' not in tag: continue
    root =  tag.replace('[', '').replace(']','') + '.txt'
    print(tag, ' --> ', root)

    cmd = 'valgrind --tool=memcheck --error-limit=no --track-origins=yes /REFPROP-tests/build/main ' + tag
    with open('/output/log_'+root,'w') as fp_stderr:
        with open('/output/err_'+root,'w') as fp_stdout:
            subprocess.run(cmd, shell = True, stdout = fp_stdout, stderr = fp_stderr)
    print(open('/output/log_'+root).readlines()[-1])

# Store all the outputs in zip archive
os.makedirs('/output/errors')
os.makedirs('/output/ok')
for g in glob.glob('/output/log_*.txt'):
    test_name = os.path.split(g)[1].replace('log_','').replace('.txt','')
    errname = f'/output/err_{test_name:s}.txt'
    is_ok = '0 errors from 0 contexts' in open(g).read()
    dest = '/output/ok' if is_ok else '/output/errors'
    for name in [g, errname]:
        shutil.move(name, dest)
shutil.make_archive('/output/errors','zip','/output/errors')
shutil.make_archive('/output/ok','zip','/output/ok')

toc = timeit.default_timer()
print('elapsed time [s]:', toc-tic)