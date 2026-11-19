from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import argparse, hashlib
p=argparse.ArgumentParser()
p.add_argument('--dll',type=Path,required=True)
a=p.parse_args()
root=Path(__file__).resolve().parents[1]
out=root/'dist';out.mkdir(exist_ok=True)
target=out/'BoHaCornerLights-1.1.0-preview-windows-x64.zip'
assert a.dll.is_file(), a.dll
with ZipFile(target,'w',ZIP_DEFLATED) as z:
    z.write(a.dll,'BoHaCornerLights.dll')
    for name in ['Install.ps1','README.md','LICENSE','THIRD_PARTY_NOTICES.md']:
        z.write(root/name,name)
    for f in (root/'package').iterdir():
        if f.is_file(): z.write(f,f.relative_to(root).as_posix())
    for name in ['minhook','scssdk']:
        f=root/'third_party'/name/'LICENSE.txt'
        z.write(f,f'licenses/{name}-LICENSE.txt')
with ZipFile(target) as z:
    assert z.testzip() is None
    assert not any(n.endswith(('.pmg','.scs')) for n in z.namelist())
digest=hashlib.sha256(target.read_bytes()).hexdigest()
(out/'SHA256SUMS.txt').write_text(f'{digest}  {target.name}\n')
print(target)
