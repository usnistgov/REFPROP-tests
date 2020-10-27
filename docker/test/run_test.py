import subprocess, os, sys, shutil, glob, timeit
from ansi2html import Ansi2HTMLConverter

tic = timeit.default_timer()
output = subprocess.run('/REFPROP-tests/build/main -t', shell = True, stdout = subprocess.PIPE).stdout.decode('utf-8')

for il, line in enumerate(output.split('\n')[1::]):

    if not line or '[' not in line: continue
    tag = '[' + line.split('[')[1]
    if 'veryslow' in tag: continue # Don't run the veryslow tests
#    if 'predef_mix' not in tag: continue
    root =  tag.replace('[', '').replace(']','') + '.txt'
    print(tag, ' --> ', root)

    cmd = 'timeout 20m /REFPROP-tests/build/main ' + tag
    with open('/output/log_'+root,'w') as fp_stderr:
        with open('/output/err_'+root,'w') as fp_stdout:
            fp_stdout.write('About to run command: '+cmd+'\n')
            fp_stdout.flush()
            subprocess.run(cmd, shell = True, stdout = fp_stdout, stderr = fp_stderr)
    loglines = open('/output/log_'+root).readlines()
    if loglines:
        print(loglines[-1])

    # Color the output log
    with open('/output/log_'+root+'.html', 'w') as fp:
        fp.write(Ansi2HTMLConverter().convert(open('/output/err_'+root).read()))

# subprocess.run('for f in log_*.txt; do echo $f && tail -n 1 $f; done', shell=True, stdout = sys.stdout, stderr = sys.stderr)

# Store all the outputs in zip archive
os.makedirs('/output/errlog')
for g in glob.glob('/output/log_*.txt') + glob.glob('/output/err_*.txt'):
    shutil.copy2(g,'/output/errlog')
shutil.make_archive('/output/output','zip','/output/errlog')

toc = timeit.default_timer()
print('elapsed time [s]:', toc-tic)