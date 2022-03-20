from torch.utils.data import Dataset
import numpy as np
import torch
import random

TSP_TYPE = [16,32,48,64,80,96,112,128]
CVRP_TYPE = [5,6,7,8,9,10]
CVRP_TIME = [6,7,8]
CVRP_CAPACITY = 100
'''
@param
path: file name you want to store
problem: if 'TSP' generate TSP else CVRP
size: how many data you want
'''
def generate_data(path, problem = 'TSP', size = 60000):
    if problem == 'TSP':
        with open(path,'w') as f:
            for _ in range(size):
                rv = random.choice(TSP_TYPE)
                f.write(str(rv)+'\n')
                arr = np.random.rand(rv,2)
                for line in range(rv):
                    f.writelines(' '.join(str(arr[line][i]) for i in range(2)))
                    f.write('\n')
    else:
        with open(path,'w') as f:
            for _ in range(size):
                cars = random.choice(CVRP_TYPE)
                nodes = cars * random.choice(CVRP_TIME)
                
                f.write(str(nodes) + ' '+ str(cars) + ' '+ str(CVRP_CAPACITY) + '\n')
                arr1 = np.random.rand(nodes,2)
                arr2 = None
                while True:
                    arr2 = np.random.randint(1,31,(nodes,1))
                    arr2[0][0] = 0.
#                     index = list(range(1,nodes))
#                     random.shuffle(index)
#                     for i in range(int(nodes/10)):
#                         arr2[i][0] *= 3
                    ratio = np.sum(arr2) / cars / CVRP_CAPACITY
                    if ratio > 0.8 and ratio < 0.95:
                        break
                
                arr = np.c_[arr1,arr2]
                for line in range(nodes):
                    f.writelines(' '.join(str(arr[line][i]) for i in range(3)))
                    f.write('\n')


class TSP_Dataset(Dataset):
    def __init__(self,path,size,max_length_req):
        # pre_processing ***.dat file as (vector,label) like
        self.data = []
        self.size = size
        self.max_length_req = max_length_req
        with open(path,'r') as f:
            for idx in range(size):
                vec = []
                first_line = f.readline()
                a = int(first_line)
                for city in range(a):
                    city_str = f.readline()
                    vec.append(list(map(float,city_str.split(' '))))
                vec = np.array(sorted(vec)).reshape(1,-1).squeeze()
                vec = np.pad(vec,(0,2 * (max_length_req - a)),'constant',constant_values=(0,0))
                self.data.append((vec,a))
                if (idx+1) % 100 == 0:
                    print('\rloading...：{0} {1}%'.format('▉'*int(10*(float(idx)/size)),
                                                  100*(float(idx)/size)), end='')
        self.data = np.array(self.data)
        print('\nDone!')
    
    def __len__(self):
        return self.size
    
    def __getitem__(self,index):
        return {'req':self.data[index][0],'label':self.data[index][1]}

    def replace_request(self, idx, request):    # request_list: [(id, request)]
        a = request[0]
        vec = np.array(request[1:]).reshape(1,-1).squeeze()
        vec = np.pad(vec,(0,2 * (self.max_length_req - a)),'constant',constant_values=(0,0))
        self.data[idx] = (vec, a)

    def get_request(self, idx):
        request, label = self.data[idx]
        return torch.Tensor(request)

    def get_label_list(self):
        return self.data[:self.size, 1].tolist()

    def extend(self, length):
        self.data = np.concatenate((self.data, np.array([([0 for j in range(self.max_length_req)], 0) for i in range(length)])),axis=0)

    def swap(self, ida, idb):
        self.data[ida], self.data[idb] = self.data[idb], self.data[ida]


class CVRP_Dataset(Dataset):
    def __init__(self,path,size,max_length_req):
        # pre_processing ***.dat file as (vector,label) like
        self.data = []
        self.size = size
        with open(path,'r') as f:
            for idx in range(size):
                vec = []
                first_line = f.readline()
                label = int(first_line.split(' ')[1])
                num_of_nodes = int(first_line.split(' ')[0])
                for city in range(num_of_nodes):
                    city_str = f.readline()
                    vec_temp = list(map(float,city_str.split(' ')))
                    vec_temp[-1] /= 30.0
                    vec.append(vec_temp)
                vec = np.array(vec).reshape(1,-1).squeeze()
                vec = np.pad(vec,(0,3 * (max_length_req - num_of_nodes)),'constant',constant_values=(0,0))
                self.data.append((vec,label))
                if (idx+1) % 100 == 0:
                    print('\rloading...：{0}{1}%'.format('▉'*int(10*(float(idx)/size)),
                                                  100*(float(idx)/size)), end='')
        self.data = np.array(self.data)
        print('\nDone!')
    
    def __len__(self):
        return self.size
    
    def __getitem__(self,index):
        return {'req':self.data[index][0],'label':self.data[index][1]}