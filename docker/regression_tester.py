import zipfile, glob, os, re

import run_one
import pandas

class ClassRunner():
    def __init__(self, *, paths, test, ofprefix):
        self.paths = paths
        self.test = test
        self.ofprefix = ofprefix
        for path in paths:
            if not os.path.exists(path):
                raise ValueError(path)

    def _build_one(self, path, **kwargs):
        print(f'Running {path} w/ hash of: ', run_one.get_path_hash(path))
        try:
            run_one.run_test(root=path, test=self.test, ofprefix=self.ofprefix, **kwargs)
        except BaseException as be:
            print(be)

    def build(self, **kwargs):
        for p in self.paths:
            self._build_one(p, **kwargs)

    def _expand_ZIP(self, *, outroot='.'):
        for path in self.paths:
            h = run_one.get_path_hash(path)
            test = self.test
            with zipfile.ZipFile(os.path.join(self.ofprefix, f'{h}_{test}.zip')) as z:
                z.extractall(path=f'{outroot}/{h}')

    def _collect_tag_list(self):
        tags = []
        for path in self.paths:
            h = run_one.get_path_hash(path)
            for f in glob.glob(h+'/errors/*.txt') + glob.glob(h+'/ok/*.txt'):
                tags.append(os.path.split(f)[1].split('.txt')[0].replace('err_',''))
        return [t for t in set(tags)] 

    def _get_tag_info(self, *, tagpath, pathhash):
        contents = open(tagpath).read()
        if 'All tests passed' in contents:
            failure_count = 0
        else:
            groups = re.search(r'assertions: (\d+) \| (\d+) passed \| (\d+) failed', contents)
            if groups is None:
                raise ValueError('no match for test result')
            failure_count = groups.group(3)
        return {
            'failure_count': failure_count,
            'url': tagpath,
            'pathhash': pathhash
        }

    def build_tag_report(self, tags, sort_by = None, sort_kwargs={}):
        o = []
        unhash = {}
        for tag in tags:
            results = []
            for path in self.paths:
                pathhash = run_one.get_path_hash(path)
                unhash[pathhash] = path 
                candidates = [os.path.join(pathhash,'errors',f'err_{tag}.txt'), os.path.join(pathhash,'ok',f'err_{tag}.txt')]
                tagpaths = [p for p in candidates if os.path.exists(p)]
                if len(tagpaths) == 1:
                    results.append(self._get_tag_info(tagpath=tagpaths[0], pathhash=pathhash))
                else:
                    print(candidates)
            if not all([r['failure_count'] == 0 for r in results]):
                # print(tag, results)
                j = {'tag': tag}
                for r in results:
                    url = r['url']
                    failure_count = r['failure_count']
                    j[unhash[r['pathhash']]] = f'<a href="{url}">{failure_count} failures</a>'
                o.append(j)
        if sort_by is not None:
            by = sort_by
        else:
            by = 'tag'
        def keyer(pair):
            if 'href' in pair.iloc[0]:
                return [int(s.split('>', 1)[-1].split('<')[0].replace(' failures', '')) for s in pair]
            else:
                return pair
            
        df = pandas.DataFrame(o).sort_values(by=by, key=keyer, **sort_kwargs)
        df.to_html('report.html', index=False, escape=False)
        df.to_csv('report.csv', index=False)
        return df

    def compare(self, sort_by = None, sort_kwargs={}, outroot=None):
        self._expand_ZIP(outroot=outroot)
        self.tags = self._collect_tag_list()
        return self.build_tag_report(self.tags, sort_by=sort_by, sort_kwargs=sort_kwargs)

if __name__ == '__main__':
    print('Can\'t run this file directly')
    quit()

    """ Example:
    from regression_tester import *
    paths = ['path1', 'path2']
    cr = ClassRunner(paths=paths, test='test')
    cr.build()
    cr.compare()
    """