import torch
import time
import threading
import numpy as np
import random
import math
import subprocess
from collections import deque

import AFEA
import afea_config


PROBLEM = afea_config.PROBLEM_TYPE
REQUEST_GENERATE_INTERVAL = afea_config.REQUEST_GENERATE_INTERVAL
BATCH_SIZE = afea_config.Config['BATCH_SIZE']
THRESHOLD = afea_config.THRESHOLD
afea = AFEA.AFEA_dqn(afea_config.Config['AFEA'])
lock = threading.Lock()
end_condition = 0


class Request_queue:
    def __init__(self, MAX_LEN=512):
        self.MAX_LENGTH = MAX_LEN
        self.queue = []
    
    def isFull(self):
        if len(self.queue) >= self.MAX_LENGTH:
            return True
        return False
    
    def getBatch(self, size):
        res = []
        if len(self.queue) < size:
            return res
        for i in range(size):
            res.append(self.queue[i])
        return res
    
    def remove(self, target_list):
        target_list.sort(reverse=True)
        for i in target_list:
            if i >= len(self.queue):
                continue
            lock.require()
            del self.queue[i]
 			lock.release()   
    def pop_front(self, size):
        while len(self.queue) > 0 and size > 0:
        	lock.require()
            self.queue.pop(0)
            size -= 1
            lock.release()
    def clear(self):
        self.queue.clear()
    
    def append_TSP(self):
        if len(self.queue) >= self.MAX_LENGTH:
            return False
        TSP_TYPE = [16,32,48,64,80,96,112,128]
        rv = random.choice(TSP_TYPE)
        arr = np.random.rand(rv,2)
        res = []
        res.append(rv)
        for i in range(rv):
            res.append(arr[i][0])
            res.append(arr[i][1])
        lock.require()
        self.queue.append([res, time.time()])
        lock.release()
        return True
            
    def append_CVRP(self):
        if len(self.queue) >= self.MAX_LENGTH:
            return False
        CVRP_TYPE = [5,6,7,8,9,10]
        CVRP_TIME = [6,7,8]
        CVRP_CAPACITY = 100
        cars = random.choice(CVRP_TYPE)
        nodes = cars * random.choice(CVRP_TIME)
        arr1 = np.random.rand(nodes,2)
        arr2 = None
        while True:
            arr2 = np.random.randint(1,31,(nodes,1))
            arr2[0][0] = 0.
            THRESHOLD = np.sum(arr2) / cars / CVRP_CAPACITY
            if THRESHOLD > 0.8 and THRESHOLD < 0.95:
                break
        arr = np.c_[arr1,arr2]
        res = []
        res.append(nodes)
        res.append(cars)
        res.append(CVRP_CAPACITY)
        for i in range(nodes):
            res.append(arr[i][0])
            res.append(arr[i][1])
            res.append(arr[i][2])
        lock.require()
        self.queue.append([res, time.time()])
        lock.release()
        return True

class Database:   # code(tensor) : [solution, fitness, id]
    def __init__(self, size=1024, cluster_num=8):
        self.size = size
        self.cluster_num = cluster_num
        self.cluster = [[] for i in range(cluster_num)]
        self.data = {}
    
    def get(self, code, label):
        if len(cluster[label]) < 1:
            return []
        res = torch.argmin(torch.sum(torch.pow(code.cpu()-torch.stack(tuple(cluster[label]), dim=0),2),dim=1)).cpu().item()
        return self.data[cluster[label][res]]

    def get_K(self, code, label):
    	if len(cluster[label]) < 1:
            return []
    	res = deque(maxlen = self.config['K_NEAREST_SOLUTIONS'])
    	for code in cluster[label]:

    
    def append(self, code, solution, fitness, idx, label):
        if len(data) >= size:
            key, value = random.choice(list(self.data.items()))
            self.cluster[label].remove(key)
            self.data.pop(key)
        self.data[code] = [solution, fitness, idx]
        self.cluster[label].append(code)
        
    def update_cluster(self):
        for x in self.cluster:
            x.clear()
        for k, v in data.items():
            label = AFEA.get_label(v[2])
            self.cluster[label].append(k)
            
    def avg_fitness(self, label):
        res = 0
        for code in cluster[label]:
            res += data[code][1]
        return res / len(cluster[label])

def write_to_file(request_batch):
	pass

def preprocessing_request_batch(request_batch):
	pass
            

def add_item_to_queue():
	while not RQ.isFull():
		if problem == 'TSP':
			RQ.append_TSP()
		else:
			RQ.append_CVRP()
		time.sleep(REQUEST_GENERATE_INTERVAL)
	global end_condition
	end_condition = 1


RQ = Request_queue(afea_config.REQUEST_QUEUE_SIZE)
DB = Database(afea_config.DB_SIZE, afea_config.Config['AutoEncoder']['num_of_clusters'])

if __name__ == '__main__':
	batch_id = 0
	eps = afea_config.EPSILON
	generate_thread = threading.Thread(target=add_item_to_queue)
	data_size = afea_config.Config['AutoEncoder']['data_size']
	for episode in range(n_episode):
		# initialization
		end_condition = 0
		RQ.clear()
		generate_thread.start()
		time_stamp = []		# id, problem, create_time, access_time, solved_time, solution, fitness
		while not end_condition:
			request_batch = RQ.getBatch(BATCH_SIZE)
			if len(request_batch) > 0:
				idx = [(batch_id * BATCH_SIZE + i)%data_size for i in range(BATCH_SIZE)]
				# record the time stamps
				for problem, stamp in request_batch:
					time_stamp.append([idx[i], problem, stamp, time.time()])
				request_batch = arrary(request_batch[:,0]).tolist()
				input_request_batch = preprocessing_request_batch(request_batch) # tensor
				codes, x_, qvalues = afea.forward(input_request_batch)
				
				request_list = [(idx[i], request_batch[i], codes[i]) for i in range(BATCH_SIZE)]
				# new requests needs to be updated to afea's dataset and cluster
				afea.update_label(request_list)
				# make decision according to q values
				action_batch = afea.epsilon_greedy_policy(qvalues,eps)

				# for problem in request_batch, solve them according corresponding action
				EA_list = []
				Lookup_list = []
				rewards = []
				for i in range(len(request_batch)):
				    if action_batch[i].item() == 0:
				        EA_list.append(request_batch[i])
				        rewards.append(THRESHOLD)
				    else:
				        res = DB.get(code[i], (batch_id * BATCH_SIZE + i)%data_size)
				        if len(res) < 1: 
				            EA_list.append(request_batch[i])
				            rewards.append(THRESHOLD)
				        else:
				        	fitness = evaluate(request_batch[i], res[0])
				        	rewards.append(fitness / res[1])
					        if fitness < res[1] * THRESHOLD:
					            EA_list.append(request_batch[i])
					        else:
					            Lookip_list.append(i)
					            time_stamp[idx[i]].append(time.time())
					            time_stamp[idx[i]].append(res[0])
					            time_stamp[idx[i]].append(fitness)
					            idx.remove(idx[i])
				RQ.remove(lookup)
				
				while RQ.size() < :
					
					sleep(1)
				# construct transition batch
				rewards = torch.Tensor(rewards)
				RQ.pop_front(len(EA_list))
				next_batch = RQ.getBatch(BATCH_SIZE)	# ??????

				# call EA to solve rest problems
				write_to_file(EA_list)
				res = call_EA(EA_path).split('\n') # solution fitness
				for i in range(EA_list):
					time_stamp[idx[i]].append(time.time())
					time_stamp[idx[i]].append(res[i][:-1])
					time_stamp[idx[i]].append(res[i][-1])
		eps *= afea_config.EPSILON_DECAY_RATE



	



