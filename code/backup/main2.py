import torch
import time
import threading
import numpy as np
import random
import math
import subprocess
from queue import PriorityQueue as PQ

import AFEA
import afea_config
import warnings
warnings.filterwarnings("ignore")

PROBLEM = afea_config.PROBLEM_TYPE
REQUEST_GENERATE_INTERVAL = afea_config.REQUEST_GENERATE_INTERVAL
BATCH_SIZE = afea_config.BATCH_SIZE
THRESHOLD = afea_config.THRESHOLD
DB_SIZE = afea_config.DB_SIZE
data_size = afea_config.Config['AutoEncoder']['data_size']
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

    def size(self):
        return len(self.queue)

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
            lock.acquire()
            del self.queue[i]
            lock.release()

    def pop_front(self, size):
        while len(self.queue) > 0 and size > 0:
            lock.acquire()
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
        for i in range(rv):
            res.append([arr[i][0], arr[i][1]])
        res = np.array(sorted(res)).reshape(1, -1).squeeze().tolist()
        res.insert(0, rv)
        lock.acquire()
        self.queue.append((res, time.time()))
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
            th = np.sum(arr2) / cars / CVRP_CAPACITY
            if th > 0.8 and th < 0.95:
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
        lock.acquire()
        self.queue.append([res, time.time()])
        lock.release()
        return True

class Database:   # code(tensor) : [solution, fitness, id]
    def __init__(self, size=1024, cluster_num=8):
        self.size = size
        self.cluster_num = cluster_num
        self.cluster_fitness = [1e9 for i in range(self.cluster_num)]
        self.data = {}
        self.id_list = {}
        # the two ptrs below indicate the next empty position in extended space(in ptr) and
        # the position of the oldest database item in extended space(out ptr)
        self.extend_in_ptr = 0
        self.extend_out_ptr = 0
        self.center = [torch.Tensor([0 for i in range(afea_config.Config['AutoEncoder']['code_length'])]).to(afea.device) for i in range(self.cluster_num)]

    def get_K(self, code):
        if len(self.data) < 1:
            return []
        K = afea_config.K_NEAREST_SOLUTIONS
        tmp = [[1e9, []] for i in range(K + 1)]
        for key, value in self.data.items():
            dis = torch.sum(torch.pow(key - code, 2)).cpu().item()
            i = K - 1
            tmp[K] = [dis, value]
            while i >= 0 and tmp[i][0] > dis:
                tmp[i], tmp[i+1] = tmp[i+1], tmp[i]
                i -= 1
        tmp.pop()
        res = []
        for x in tmp:
            res.append(x[1])
        return res

    def get_center(self, label):
        return self.center[label]

    def append(self, code, solution, fitness, idx):
        if len(self.data) >= self.size:

            if self.extend_in_ptr == self.extend_out_ptr:
                # two ptr points to same position indicates that there is no database item in extended space
                # randomly choose an item to pop
                key, value = random.choice(list(self.data.items()))
                self.data.pop(self.id_list.pop(value[2]))
            else:
                # there are some database items in the extended space, they should be replace preferentially
                self.data.pop(self.id_list.pop(self.extend_out_ptr + data_size))
                self.extend_out_ptr = (self.extend_out_ptr + 1)%DB_SIZE
        self.data[code] = [solution, fitness, idx]
        self.id_list[idx] = code

    def update_cluster(self):
        self.cluster_fitness = [0.0 for i in range(self.cluster_num)]
        # fitness_list = [[] for i in range(self.cluster_num)]
        cluster_size = [0.0 for i in range(self.cluster_num)]
        self.center = [torch.Tensor([0 for i in range(afea_config.Config['AutoEncoder']['code_length'])]).to(afea.device) for i in range(self.cluster_num)]
        for idx, code in self.id_list.items():
            label = afea.get_label(idx)
            new_code = afea.predict_codes(afea.autoencoder.dataset.get_request(idx).to(afea.device))
            value = self.data[code]     # key: code(tensor),  value: [solution, fitness, id]
            # fitness_list[label].append(value[1])
            self.cluster_fitness[label] += value[1]
            cluster_size[label] += 1
            del self.data[code]
            self.data[new_code] = value
            self.id_list[idx] = new_code
            self.center[label] = self.center[label] + new_code
        for i in range(self.cluster_num):
            self.center[i] = self.center[i] / len(self.data)
        for i in range(self.cluster_num):
            # if len(fitness_list[i]) > 0:
            if cluster_size[i] > 0:
                self.cluster_fitness[i] /= cluster_size[i]
                # self.cluster_fitness[i] = np.median(fitness_list[i])
            else:
                self.cluster_fitness[i] = 1e9


    def avg_fitness(self, label):
        return self.cluster_fitness[label]

def TSP_evaluate(problem, solution):
    res = 0
    dim = problem[0]
    if dim > len(solution):
        return dim
    path= []
    for i in solution:
        if i <= dim:
            path.append(i)
    for i in range(dim):
        a = int(path[i])
        b = int(path[(i + 1)%dim])
        xa = problem[a * 2 - 1]
        ya = problem[a * 2]
        xb = problem[b * 2 - 1]
        yb = problem[b * 2]
        res += math.sqrt((xa - xb) * (xa - xb) + (ya - yb) * (ya - yb))
    return res

def CVRP_evaluate(problem, solution):
    dim = problem[0]
    cars = problem[1]
    capacity = problem[2]
    if dim > len(solution):
        return 1e9
    path = []
    for i in solution:
        if i <= dim:
            path.append(i)
    problem = problem[3:]
    def dis(a, b):
        xa = problem[a * 3]
        ya = problem[a * 3 + 1]
        xb = problem[b * 3]
        yb = problem[b * 3 + 1]
        return math.sqrt((xa - xb) * (xa - xb) + (ya - yb) * (ya - yb))

    dp = [[1e9] * (cars + 1) for i in range(dim + 1)]
    de = [[1e9] * (cars + 1) for i in range(dim + 1)]
    dp[1][1] = dis(0, path[0]) + dis(path[0], 0)
    de[1][1] = problem[path[0] * 3 + 2]
    for i in range(1, dim + 1):
        for j in range(1, min(i, cars) + 1):
            if de[i - 1][j] + problem[path[i] * 3 + 2] <= capacity:
                if dp[i-1][j] - dis(path[i-1], 0) + dis(path[i-1], path[i]) + dis(path[i], 0) <= dp[i-1][j-1] + dis(path[i], 0) + dis(0, path[i]):
                    dp[i][j] = dp[i - 1][j] - dis(path[i - 1], 0) + dis(path[i - 1], path[i]) + dis(path[i], 0)
                    de[i][j] = de[i - 1][j] + problem[path[i] * 3 + 2]
                else:
                    dp[i][j] = dp[i - 1][j - 1] + dis(path[i], 0) + dis(0, path[i])
                    de[i][j] = problem[path[i] * 3 + 2]
            else:
                dp[i][j] = dp[i - 1][j - 1] + dis(path[i], 0) + dis(0, path[i])
                de[i][j] = problem[path[i] * 3 + 2]
    return dp[dim][cars]

def evaluate(problem, solution, Type="TSP"):
    if Type == "TSP":
        return TSP_evaluate(problem, solution)
    else:
        return CVRP_evaluate(problem, solution)

def write_to_file(request_batch, path):
    with open(path,'w') as f:
        for request in request_batch:
            f.writelines(' '.join(str(i) for i in request))
            f.write('\n')


def preprocessing_request_batch(request_batch):
    res = []
    for request in request_batch:
        a = request[0]
        vec = np.array(request)[1:].reshape(1,-1).squeeze()
        vec = np.pad(vec,(0,2 * (afea.autoencoder.dataset.max_length_req - a)),'constant',constant_values=(0,0))
        res.append(vec)
    return torch.Tensor(res)

def add_item_to_queue():
    while not RQ.isFull():
        if PROBLEM == 'TSP':
            RQ.append_TSP()
        else:
            RQ.append_CVRP()
        time.sleep(REQUEST_GENERATE_INTERVAL)
    global end_condition
    end_condition = 1

# class Generate_Thread(threading.Thread):
#     """Thread class with a stop() method. The thread itself has to check
#     regularly for the stopped() condition."""
#
#     def __init__(self):
#         super(Generate_Thread, self).__init__()
#         self._stop_event = threading.Event()
#
#     def stop(self):
#         self._stop_event.set()
#
#     def run(self):
#         while not self._stop_event.isSet():
#             if not RQ.isFull():
#                 if PROBLEM == 'TSP':
#                     RQ.append_TSP()
#                 else:
#                     RQ.append_CVRP()
#                 print('new request generated')
#                 time.sleep(REQUEST_GENERATE_INTERVAL)
#             else:
#                 global end_condition
#                 end_condition = 1


def call_EA(path, num_problem):
    p = subprocess.Popen([path, str(num_problem)], stdout=subprocess.PIPE)
    return p

def update_label(request_list):
    global data_size
    for idx, req, code in request_list:
        # if the idx exist in database, directly replacing will make the id of database item invalid
        if idx in DB.id_list:
            # put the database item into extended space
            afea.autoencoder.dataset.replace_request(DB.extend_in_ptr + data_size, req)
            afea.autoencoder.dataset.swap(idx, DB.extend_in_ptr + data_size)

            # update id changing information in databse
            DB.data[DB.id_list[idx]][2] = DB.extend_in_ptr + data_size
            DB.id_list[DB.extend_in_ptr + data_size] = DB.id_list.pop(idx)

            # update assignment information in autoencoder
            afea.autoencoder.assignments[DB.extend_in_ptr + data_size] = afea.autoencoder.assignments[idx]

            # ptr points to next position that can contain next replaced database item
            DB.extend_in_ptr = (DB.extend_in_ptr + 1)%DB_SIZE

            # if the size of dataset is not maximal(data size + DB size), which means in above process, a new item is appended
            if len(afea.autoencoder.dataset) < data_size + DB_SIZE:
                afea.autoencoder.dataset.size += 1
                afea.autoencoder.data_size += 1

            # get and update clustering information for this req
            min_dis_idx = torch.argmin(torch.sum(torch.pow(code.cpu() - afea.autoencoder.centers, 2), dim=1)).cpu().item()
            afea.autoencoder.assignments[idx] = min_dis_idx
            afea.autoencoder.npoints[min_dis_idx] += 1

        # if the idx is not exist in database, directly replace
        else:
            afea.autoencoder.dataset.replace_request(idx, req)

            # get and update clustering information for this req
            min_dis_idx = torch.argmin(torch.sum(torch.pow(code.cpu()-afea.autoencoder.centers,2),dim=1)).cpu().item()
            afea.autoencoder.npoints[afea.autoencoder.assignments[idx]] -= 1
            afea.autoencoder.assignments[idx] = min_dis_idx
            afea.autoencoder.npoints[min_dis_idx] += 1

RQ = Request_queue(afea_config.REQUEST_QUEUE_SIZE)
DB = Database(DB_SIZE, afea_config.Config['AutoEncoder']['num_of_clusters'])

if __name__ == '__main__':
    batch_id = 0
    eps = afea_config.EPSILON


    # extend dataset size and assignment records size in afea for database items
    afea.autoencoder.dataset.extend(DB_SIZE)
    afea.autoencoder.extend_assignments(DB_SIZE)

    for episode in range(afea_config.N_EPISODE):
        # data record ====================================================
        num_solved_request = 0
        episode_start = time.time()
        loss_list = []
        RQ_size = []
        num_lookup_succ = []
        num_lookup_fail = []
        num_EA = []
        dim_error = 0
        time_stamp = {} # id, problem, create_time, cluster, access_time, solved_time, solution, fitness
        # data record ====================================================

        # initialization
        end_condition = 0
        RQ.clear()
        generate_thread = threading.Thread(target=add_item_to_queue)
        generate_thread.start()

        while not end_condition:
            data_batch = RQ.getBatch(BATCH_SIZE)
            request_batch = []
            if len(data_batch) > 0:
                idx = [(batch_id * BATCH_SIZE + i)%data_size for i in range(BATCH_SIZE)]

                # record the time stamps
                for i in range(len(data_batch)):
                    problem, stamp = data_batch[i]
                    # data record ====================================================
                    time_stamp[idx[i]] = [idx[i], problem[0], problem, stamp, time.time()]
                    # data record ====================================================
                    request_batch.append(problem)

                # construct request batch and get codes, qvalues from AFEA

                input_request_batch = preprocessing_request_batch(request_batch)  # tensor
                codes = afea.predict_codes(input_request_batch.to(afea.device))

                # new requests needs to be updated to afea's dataset and cluster
                request_list = [(idx[i], request_batch[i], codes[i]) for i in range(BATCH_SIZE)]
                update_label(request_list)
                label_batch = []
                for i in idx:
                    label = afea.autoencoder.assignments[i]
                    label_batch.append(label)

                # codes, qvalues = afea.forward(input_request_batch)
                states = []
                for i in range(BATCH_SIZE):
                    states.append(codes[i] - DB.get_center(label_batch[i]).to(afea.device))
                states = torch.stack(states, 0)
                qvalues = afea.predict_q_value(states)

                # make decision according to q values
                action_batch = afea.epsilon_greedy_policy(qvalues,eps)

                # for problem in request_batch, solve them according corresponding action
                EA_list = []
                Lookup_list = []
                rewards = []
                code_for_DB = []
                # data record ====================================================
                num_EA.append(0)
                num_lookup_fail.append(0)
                num_lookup_succ.append(0)
                # data record ====================================================
                for i in range(len(request_batch)):
                    if action_batch[i] == 1:
                        EA_list.append(request_batch[i])
                        rewards.append(THRESHOLD)
                        code_for_DB.append(codes[i].clone())
                        # data record ====================================================
                        num_EA[-1] += 1
                        # data record ====================================================
                    else:
                        res = DB.get_K(codes[i].clone())
                        if len(res) < 1:    # lookup fail due to empty database
                            EA_list.append(request_batch[i])
                            code_for_DB.append(codes[i].clone())
                            rewards.append(THRESHOLD)
                            # data record ====================================================
                            num_lookup_fail[-1] += 1
                            # data record ====================================================
                        else:
                            # get the best performance solution from K nearest items
                            best = []
                            fitness = 0
                            for value in res:
                                f = 1 / evaluate(request_batch[i], value[0], PROBLEM)
                                if f > fitness:
                                    fitness = f
                                    best = value
                            res = best
                            if fitness < 0.0001:
                                dim_error += 1
                            # print(label_batch[i], request_batch[i][0], 1/fitness, 1 / DB.avg_fitness(label_batch[i]))
                            # determine whether the solution is good enough
                            ratio = fitness / DB.avg_fitness(label_batch[i])
                            # rewards.append(ratio)
                            if ratio < THRESHOLD:
                                rewards.append(ratio)
                                EA_list.append(request_batch[i])
                                code_for_DB.append(codes[i])
                                # data record ====================================================
                                num_lookup_fail[-1] += 1
                                # data record ====================================================
                            else:
                                rewards.append(ratio)
                                Lookup_list.append(i)
                                time_stamp[idx[i]].append(time.time())
                                time_stamp[idx[i]].append(res[0])
                                time_stamp[idx[i]].append(fitness)
                                # data record ====================================================
                                num_lookup_succ[-1] += 1
                                # data record ====================================================


                # lookup requests are solved, remove them from RQ
                for i in Lookup_list:
                    idx.remove((batch_id * BATCH_SIZE + i)%data_size)
                RQ.remove(Lookup_list)

                # call EA to solve rest problems
                write_to_file(EA_list, afea_config.EA_DATA_PATH)
                MFEA_start_time = time.time()
                p = call_EA(afea_config.EA_PATH, len(EA_list))

                # update database for next state
                for i in range(len(code_for_DB)):
                    DB.append(code_for_DB[i], [], 0, idx[i])
                DB.update_cluster()

                # construct transition batch
                next_batch = RQ.getBatch(BATCH_SIZE)
                while len(next_batch) < 1:      # if the size of RQ is not enough for a batch
                    # update database and time stamp record
                    time.sleep(REQUEST_GENERATE_INTERVAL)
                    next_batch = RQ.getBatch(BATCH_SIZE)

                next_batch = preprocessing_request_batch(np.array(RQ.getBatch(BATCH_SIZE))[:,0]).to(afea.device)
                next_codes = afea.predict_codes(next_batch)
                transition_batch = []
                is_done = True if end_condition else False
                for i in range(BATCH_SIZE):
                    state = input_request_batch[i]
                    action = action_batch[i]
                    next_label = torch.argmin(torch.sum(torch.pow(next_codes[i] - torch.stack(DB.center, 0), 2), dim=1)).cpu().item()
                    next_state = next_codes[i] - DB.get_center(next_label).to(afea.device)
                    reward = rewards[i]
                    transition_batch.append((state, action, next_state, reward, is_done))
                afea.update_memory(transition_batch)

                # AFEA learning
                reconstruction_loss, clustering_loss, q_loss, clustering_entropy = afea.learning([DB.get_center(i) for i in range(afea.autoencoder.NUM_OF_CLUSTER)])
                # data record ====================================================
                loss_list.append((reconstruction_loss, clustering_loss, q_loss, clustering_entropy))

                # data record ====================================================
                print('************************************************************************************')
                print('Episode {} Batch {}:'.format(episode,batch_id))
                print('rloss = {}, closs = {}, qloss = {}'.format(reconstruction_loss,clustering_loss,q_loss))
                print('Clustering Entropy: {}'.format(clustering_entropy))
                print('Lookup Succ: {}, Lookup Fail: {}, EA: {}'.format(num_lookup_succ[-1], num_lookup_fail[-1], num_EA[-1]))

                # update database and time stamp record

                res, err = p.communicate()

                # EA requests are solving, remove them from RQ
                RQ.pop_front(len(EA_list))

                MFEA_end_time = time.time()
                print('MFEA solving time: {}'.format(MFEA_end_time - MFEA_start_time))
                res = str(res)[2:-5].split('\\r\\n')
                for i in range(len(res)):
                    res[i] = list(map(float, res[i].split()))
                    # data record ====================================================
                    time_stamp[idx[i]].append(time.time())
                    time_stamp[idx[i]].append(res[i][:-1])
                    time_stamp[idx[i]].append(res[i][-1])
                    # data record ====================================================
                    DB.data[DB.id_list[idx[i]]][0] = res[i][:-1]
                    DB.data[DB.id_list[idx[i]]][1] = 1 / res[i][-1]
                DB.update_cluster()
                num_solved_request += BATCH_SIZE
                print('Avg Fitness: ', end='')
                for i in range(afea.autoencoder.NUM_OF_CLUSTER):
                    print('{} '.format(1 / DB.avg_fitness(i)), end='')
                print()
                afea.autoencoder.print_dlist()
                print('Waiting Requests: {}'.format(RQ.size()))
                print('Database size: {}'.format(len(DB.data)))
                print('************************************************************************************')
                print()
                RQ_size.append(RQ.size())
                batch_id+=1

        eps *= afea_config.EPSILON_DECAY_RATE

        # data record
        episode_end = time.time()
        episode_duration = episode_end - episode_start
        delay_list = []
        fitness_per_cluster = [0] * afea.autoencoder.NUM_OF_CLUSTER
        num_per_cluster = [0] * afea.autoencoder.NUM_OF_CLUSTER
        for idx, value in time_stamp.items():
            # id, label, problem, create_time, access_time, solved_time, solution, fitness
            id, label, problem, create_time, access_time, solved_time, solution, fitness = value
            delay_list.append(solved_time - create_time)
            fitness_per_cluster[label // 16 - 1] += fitness
            num_per_cluster[label // 16 - 1] += 1
        for i in range(afea.autoencoder.NUM_OF_CLUSTER):
            fitness_per_cluster[i] /= num_per_cluster[i]
        print('-------------------------------------------------------------------------------------------------------')
        print('Episode {}:\nThroughput: {}\nSolving Delay: {}'.format(episode,num_solved_request/episode_duration,sum(delay_list)/len(delay_list)))
        print('Fitness Per Type: ', end = '')
        for i in range(len(fitness_per_cluster)):
            print('{} '.format(fitness_per_cluster[i]), end='')
        print()
        print('Look up Succ: {}, Look up Fail: {}, Direct Using EA: {}, Dim Error: {}'.format(sum(num_lookup_succ),sum(num_lookup_fail),sum(num_EA), dim_error))
        print('Episode Duration: {}'.format(episode_duration))

        print('-------------------------------------------------------------------------------------------------------')
        print()









