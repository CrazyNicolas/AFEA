import subprocess


res = subprocess.getoutput('Internal TSP16.txt Internal_1')
# res = list(map(int, res.split()))
print(res)
