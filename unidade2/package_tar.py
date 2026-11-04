import os
import tarfile
import shutil

base_dir = os.path.dirname(os.path.abspath(__file__))
lgbm_dir = os.path.join(base_dir, 'lgbm')
tar_path = os.path.join(base_dir, 'lgbm.tar')

# Files to include in tar
included_files = ['sync_car.c', 'sync_car.h', 'main.c', 'Makefile', 'README.md']

with tarfile.open(tar_path, 'w') as tar:
    for fname in included_files:
        fpath = os.path.join(lgbm_dir, fname)
        arcname = f"lgbm/{fname}"
        tar.add(fpath, arcname=arcname)
        print(f"Added to tar: {arcname}")

print(f"Archive created at: {tar_path}")

# Copy to root as well
root_dir = os.path.dirname(base_dir)
root_tar = os.path.join(root_dir, 'lgbm.tar')
shutil.copy2(tar_path, root_tar)
print(f"Copied to root: {root_tar}")
