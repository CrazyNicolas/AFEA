import sys  
sys.path.append("..")
import torch
import torch.nn.functional as f
import time
import random
from torch.utils.data import DataLoader
from data import DataSet as DS
import afea_config
import math
from collections import defaultdict


class Encoder(torch.nn.Module):
    def __init__(self,input_size,code_layer_size):
        super(Encoder,self).__init__()
        #self.ln1 = torch.nn.Linear(input_size,1000)
        self.ln2 = torch.nn.Linear(input_size,512)
        self.ln3 = torch.nn.Linear(512,128)
        self.ln4 = torch.nn.Linear(128,code_layer_size)
    
    def forward(self,x):
        #x = f.relu(self.ln1(x))
        x1 = f.selu(self.ln2(x))
        x2 = f.selu(self.ln3(x1))
        y = f.selu(self.ln4(x2))
        return y

class Decoder(torch.nn.Module):
    def __init__(self,out_size,code_layer_size):
        super(Decoder,self).__init__()
        self.ln1 = torch.nn.Linear(code_layer_size,128)
        self.ln2 = torch.nn.Linear(128,512)
        self.ln3 = torch.nn.Linear(512,out_size)
        #self.ln4 = torch.nn.Linear(1000,out_size)
    def forward(self,x):
        x1 = f.selu(self.ln1(x))
        x2 = f.selu(self.ln2(x1))
        #x = f.relu(self.ln3(x))
        y = f.relu(self.ln3(x2))
        return y


class AutoEncoder():
    def __init__(self,config,writter):
        self.writter = writter
        self.bsize = config['batch_size']
        self.CODE_LENGTH = config['code_length']
        self.device = torch.device('cuda:0' if torch.cuda.is_available() else 'cpu')
        self.NUM_OF_CLUSTER = config['num_of_clusters']
        self.data_size = config['data_size']
        self.dataset = DS.TSP_Dataset(config['data_source'], config['data_size'], config['max_cites'])
        self.loader = DataLoader(self.dataset,shuffle=False,batch_size = self.bsize)
        self.encoder = Encoder(2 * config['max_cites'] if config['problem_type'] == 'TSP' else 3 * config['max_cites'],self.CODE_LENGTH)
        self.decoder = Decoder(2 * config['max_cites'] if config['problem_type'] == 'TSP' else 3 * config['max_cites'],self.CODE_LENGTH)
        self.autoencoder = torch.nn.Sequential(self.encoder,self.decoder).to(self.device)
        self.optimizer = torch.optim.Adamax(self.autoencoder.parameters(),lr=config['lr'])
        self.criterion = torch.nn.MSELoss().to(self.device)
        self.pre_epoch = config['pre_train_epoch']
        self.cluster_epoch = config['clustering_train_epoch']
        self.assignments = []
        self.centers = []
        self.npoints = {}
        




    def calculate_entropy(self):
        label_per_cluster = [{} for _ in range(self.NUM_OF_CLUSTER)]
        dataset_label = self.dataset.get_label_list()
        cluster_count = [0] * self.NUM_OF_CLUSTER
        for i in range(self.data_size):
            assign = self.assignments[i]
            cluster_count[assign] += 1
            if label_per_cluster[assign].__contains__(dataset_label[i] // 16 - 1):
                label_per_cluster[assign][dataset_label[i] // 16 - 1] += 1
            else:
                label_per_cluster[assign][dataset_label[i] // 16 - 1] = 1
        res = 0
        for i in range(self.NUM_OF_CLUSTER):
            cluster_res = 0
            for k, v in label_per_cluster[i].items():
                cluster_res += - (v/cluster_count[i]) * math.log2(v/cluster_count[i])
            res += cluster_count[i] / self.data_size * cluster_res
        return res


    def pre_train(self):
        for epoch in range(self.pre_epoch):
            loss = 0
            for bid,data in enumerate(self.loader):
                reqs = data['req'].to(self.device).float()
                preds = self.autoencoder(reqs)
                batch_loss = self.criterion(reqs,preds)
                loss += batch_loss.item()
                self.optimizer.zero_grad()
                batch_loss.backward()
                self.optimizer.step()
            print('EPOCH {},loss = {}'.format(epoch + 1, loss/math.ceil(self.data_size / self.bsize)))
            for name, layer in self.autoencoder.named_parameters():
                self.writter.add_histogram(name + '_weight_AE_PRE', layer.cpu().data.numpy(), epoch)
                self.writter.add_histogram(name + '_grad_AE_PRE', layer.grad.cpu().data.numpy(), epoch)
        #saving pre model
        torch.save(self.autoencoder, '../model/pre_ae' + str(time.time()) + '.pkl')
        

    #pretrain结束了 现在要加入聚类来进行训练
    # 正式训练的时候 要让dataloader不shuffle shuffle的话一切都乱了
    #loader = DataLoader(train_data,shuffle=False,batch_size = self.bsize)
    # 先随机选择中心初始化
    # 初始化聚类中心 Mnist分为10个类

    def init_cluster(self):
        codes = []
        # find code layer representation for ALL  data
        for bid,data in enumerate(self.loader):
            with torch.no_grad():
                outputs = self.encoder(data['req'].to(self.device).float()).cpu()
                codes.append(outputs)
        codes = torch.cat(codes,0).to(self.device)  
        rv = random.randint(0,self.data_size)
       
        for i in range(0,self.NUM_OF_CLUSTER):
            self.npoints[i] = 0.
        # 初始化中心
        for i in range(self.NUM_OF_CLUSTER):
            self.centers.append(codes[(rv + i * i)%self.data_size].tolist())
        self.centers = torch.Tensor(self.centers)
        # 在初始化了十个中心之后  可以利用 codes和这十个中心来找最小值,最小的那个距离就是对应的center就是赋值
        for code in codes:
            min_dis_idx = torch.argmin(torch.sum(torch.pow(code.cpu()-self.centers,2),dim=1)).cpu().item()
            self.assignments.append(min_dis_idx)
            self.npoints[min_dis_idx] += 1
       

    def learning(self,return_loss = False):
        # 还是按batch来做
        rloss = 0
        closs = 0
        for bid,data in enumerate(self.loader):
            data = data['req'].to(self.device).float()
            codes_preds = self.encoder(data)
            codes = codes_preds.cpu()
            preds = self.decoder(codes_preds)

            # 第一部分损失
            reconstruction_loss = self.criterion(data,preds)
            # 计算第二部分，聚类损失
            # 构建聚类中心列表 转换为Tensor
            c_y = []
            for code in codes:
                min_dis_idx = torch.argmin(torch.sum(torch.pow(code-self.centers,2),dim=1)).cpu().item()
                c_y.append(self.centers[min_dis_idx].tolist())
            c_y = torch.Tensor(c_y).to(self.device)
            clustering_loss = self.criterion(codes_preds,c_y)

            rloss = rloss +  reconstruction_loss
            closs = closs + clustering_loss
            if return_loss:
                continue
            # 先不设置权重了 感觉原论文的建议也不一定是靠谱的
            #loss = clustering_loss
            self.optimizer.zero_grad()
            loss =  reconstruction_loss + clustering_loss

            loss.backward()
            self.optimizer.step()
        rloss /= math.ceil(self.data_size / self.bsize)
        closs /= math.ceil(self.data_size / self.bsize)


        if return_loss:
            return rloss,closs
        print('rloss = {}, closs={}'.format(rloss.item(), closs.item()))

    def update_centers(self):
        for i in range(self.NUM_OF_CLUSTER):
            for j in range(self.CODE_LENGTH):
                self.centers[i][j] = 0
        # 每learning一次  就要更新一次 更新的逻辑是上一次的簇里那些点现在的code的中心
        for bid,data in enumerate(self.loader):
            data = data['req'].to(self.device).float()
            codes = None
            with torch.no_grad():
                codes = self.encoder(data).cpu()
            for idx,code in enumerate(codes):
                self.centers[self.assignments[bid * self.bsize + idx]] += code
        # 所有batch走完 每个点都已经入库  现在就是把centers做好更新,即求均值作为中心
        for i in range(0,self.NUM_OF_CLUSTER):
            self.centers[i] = (self.centers[i] / self.npoints[i])
        #return self.centers
            

    def reassignment(self):
        # 需要做的工作是 按照目前每个data的code 去找最近的新的聚类中心 然后改变a和n
        #self.assignments = [0] * 60000
        #self.npoints = {}
        for i in range(self.NUM_OF_CLUSTER):
            self.npoints[i] = 0.
        for bid,data in enumerate(self.loader):
            data = data['req'].to(self.device).float()
            codes = None
            with torch.no_grad():
                codes = self.encoder(data).cpu()
            for idx,code in enumerate(codes):
                min_dis_idx = torch.argmin(torch.sum(torch.pow(code-self.centers,2),dim=1)).cpu().item()
                self.assignments[bid * self.bsize + idx] = min_dis_idx
                self.npoints[min_dis_idx] += 1
        #return self.assignments,self.npoints
    

    def print_dlist(self):
        dlist = []
        for i in range(self.NUM_OF_CLUSTER):
            dlist.append(defaultdict(int))
        targets = self.dataset.data[:, 1]
        for idx in range(self.data_size):
            dlist[self.assignments[idx]][targets[idx]] += 1
        print(dlist)
    
    # 初始化结束了以后 开始训练的时候按照paper中的伪代码部分执行
    def clustering_train(self):
        for epoch in range(self.cluster_epoch):
            # 更新AE，通过重建和聚类损失的加和反向传播
            print('-'*60)
            print('epoch {}'.format(epoch))
            start = time.time()
            self.learning()
            for name, layer in self.autoencoder.named_parameters():
                self.writter.add_histogram(name + '_weight_AE_CLUS', layer.cpu().data.numpy(), epoch)
                self.writter.add_histogram(name + '_grad_AE_CLUS', layer.grad.cpu().data.numpy(), epoch)
            e1 = time.time()
            # 更新聚类中心
            #self.centers = update_centers()
            self.update_centers()
            e2 = time.time()
            # 给所有数据重新赋聚类中心
            #self.assignments,self.npoints = reassignment()
            self.reassignment()
            end = time.time()

            print('entropy: {}'.format(self.calculate_entropy()))

            self.print_dlist()
            #print('epoch {} 结束， learning用时：{}, updating用时：{}, reassignment用时：{}'.format(epoch,e1-start,e2-e1,end-e2))
            #print('self.centers: {}'.format(self.centers))
        torch.save(self.autoencoder, '../model/AE' + str(time.time()) + '.pkl')
        

    def train(self):
        
        self.pre_train()
        self.init_cluster()
        self.clustering_train()

    def extend_assignments(self, length):
        for i in range(length):
            self.assignments.append(0)


if __name__ == '__main__':
    autoencoder = AutoEncoder(afea_config.Config['AutoEncoder'])
    autoencoder.train()