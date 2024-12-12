from git import Repo

repo = Repo('../../REFPROP-sandbox')
commit = repo.heads.master.commit

with open(f'{commit.authored_date}_{commit.hexsha}.zip', 'wb') as fp:
    repo.archive(fp, commit.hexsha, format='zip')