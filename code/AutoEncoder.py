import torch
from torchvision import datasets,transforms
import os
from torch.utils.data import DataLoader
import matplotlib.pyplot as plt
import random
import time


# TEST using MNIST dataset
train_data = datasets.MNIST('./mnist',train=True,download=True,transform = transforms.ToTensor())
test_data = datasets.MNIST('./mnist',train=False,download=True,transform = transforms.ToTensor())
class Encoder(torch.nn.Module):
    def __init__(self,input_size,code_layer_size):
        super(Encoder,self).__init__()
        self.ln1 = torch.nn.Linear(input_size,1000)
        self.ln2 = torch.nn.Linear(1000,250)
        self.ln3 = torch.nn.Linear(250,50)
        self.ln4 = torch.nn.Linear(50,code_layer_size)
    
    def forward(self,x):
        x = x.view(-1,784)
        x = torch.relu(self.ln1(x))
        x = torch.relu(self.ln2(x))
        x = torch.relu(self.ln3(x))
        return torch.relu(self.ln4(x))

class Decoder(torch.nn.Module):
    def __init__(self,out_size,code_layer_size,reshape = False):
        super(Decoder,self).__init__()
        self.ln1 = torch.nn.Linear(code_layer_size,50)
        self.ln2 = torch.nn.Linear(50,250)
        self.ln3 = torch.nn.Linear(250,1000)
        self.ln4 = torch.nn.Linear(1000,out_size)
        self.reshape = reshape
    def forward(self,x):
        x = torch.relu(self.ln1(x))
        x = torch.relu(self.ln2(x))
        x = torch.relu(self.ln3(x))
        x = torch.relu(self.ln4(x))
        if self.reshape:
            x = x.resize(28,28)
        return x

# Some basic config
bsize = 200
loader = DataLoader(train_data,shuffle=False,batch_size = bsize)
encoder = Encoder(784,10)
decoder = Decoder(784,10)
device = torch.device('cuda:0' if torch.cuda.is_available() else 'cpu')  
autoencoder = torch.nn.Sequential(encoder,decoder).to(device)
print(device)
optimizer = torch.optim.Adam(autoencoder.parameters(),lr=1e-3)
criterion = torch.nn.MSELoss().to(device)

# Pretrainning is generally useful, yet you can ignore it 
for epoch in range(100):
    for bid,(data,_) in enumerate(loader):
        data = data.to(device)
        preds = autoencoder(data)
        loss = criterion(data.view(-1,784),preds)
        if (bid+1) % 20== 0:
            print('EPOCH {},loss = {}'.format(epoch+1,loss.item()))
        optimizer.zero_grad()
        loss.backward()
        optimizer.step()


# find code layer representation for ALL MNIST data
codes = []
for bid,(data,_) in enumerate(loader):
    with torch.no_grad():
        outputs = encoder(data.to(device)).cpu()
        codes.append(outputs)
codes = torch.cat(codes,0).to(device)

# find mean and std of these representation
mean = torch.mean(codes,0)
std = torch.std(codes,0)

# pretrain结束了 现在要加入聚类来进行训练
# 正式训练的时候 要让dataloader不shuffle shuffle的话一切都乱了
#loader = DataLoader(train_data,shuffle=False,batch_size = bsize)
# 先随机选择中心初始化
# 初始化聚类中心 Mnist分为10个类
import time
def init_cluster(N,num_of_cluster):
    assignments = []
    centers = []
    # 初始化一个每一个类有多少个点的字典
    npoints = {}
    for i in range(0,10):
        npoints[i] = 0.
    # 初始化中心
    for i in range(num_of_cluster):
        centers.append(torch.normal(mean,std).tolist())
    centers = torch.Tensor(centers)
    # 在初始化了十个中心之后  可以利用 codes和这十个中心来找最小值,最小的那个距离就是对应的center就是赋值
    for code in codes:
        min_dis_idx = torch.argmin(torch.sum(torch.pow(code.cpu()-centers,2),dim=1)).cpu().item()
        assignments.append(min_dis_idx)
        npoints[min_dis_idx] += 1
    # 这样就初始化好了 返回这些必要信息就好了
    return assignments,centers,npoints

def learning():
    # 还是按batch来做
    print('开始学习')
    for bid,(data,_) in enumerate(loader):
        data = data.view(-1,784).to(device)
        codes_preds = encoder(data)
        codes = codes_preds.cpu()
        preds = decoder(codes_preds)
        optimizer.zero_grad()
        # 第一部分损失
        reconstruction_loss = criterion(data,preds)
        # 计算第二部分，聚类损失
        # 构建聚类中心列表 转换为Tensor
        c_y = []
        for code in codes:
            min_dis_idx = torch.argmin(torch.sum(torch.pow(code-centers,2),dim=1)).cpu().item()
            c_y.append(centers[min_dis_idx].tolist())
        c_y = torch.Tensor(c_y).to(device)
        clustering_loss = criterion(codes_preds,c_y)
        # 先不设置权重了 感觉原论文的建议也不一定是靠谱的
        #loss = clustering_loss
        loss = reconstruction_loss + clustering_loss
        #print('loss = {}, closs={}'.format(loss.item(),clustering_loss.item()))
        print('loss = {}, rloss = {}, closs={}'.format(loss.item(),reconstruction_loss.item(),clustering_loss.item()))
        loss.backward()
        optimizer.step()

def update_centers():
    print('学习完成，开始更新聚类中心')
    centers = torch.zeros(10,10)
    # 每learning一次  就要更新一次 更新的逻辑是上一次的簇里那些点现在的code的中心
    for bid,(data,_) in enumerate(loader):
        data = data.view(-1,784).to(device)
        codes = None
        with torch.no_grad():
            codes = encoder(data).cpu()
        for idx,code in enumerate(codes):
            centers[assignments[bid * 200 + idx]] += code
    # 所有batch走完 每个点都已经入库  现在就是把centers做好更新,即求均值作为中心
    for i in range(0,10):
        centers[i] = (centers[i] / npoints[i])
    return centers
            

def reassignment():
    print('重新聚类并更新具体赋值')
    # 需要做的工作是 按照目前每个data的code 去找最近的新的聚类中心 然后改变a和n
    assignments = [0] * 60000
    npoints = {}
    for i in range(10):
        npoints[i] = 0.
    for bid,(data,_) in enumerate(loader):
        data = data.view(-1,784).to(device)
        codes = encoder(data).cpu()
        for idx,code in enumerate(codes):
            min_dis_idx = torch.argmin(torch.sum(torch.pow(code-centers,2),dim=1)).cpu().item()
            assignments[bid * 200 + idx] = min_dis_idx
            npoints[min_dis_idx] += 1
    return assignments,npoints
    
    
    
# 初始化结束了以后 开始训练的时候按照paper中的伪代码部分执行
def clustering_train(n_epoch = 60):
    for epoch in range(n_epoch):
        # 更新AE，通过重建和聚类损失的加和反向传播
        print('-'*60)
        print('epoch {} 开始'.format(epoch))
        start = time.time()
        learning()
        # 更新聚类中心
        centers = update_centers()
        # 给所有数据重新赋聚类中心
        assignments,npoints = reassignment()
        end = time.time()
        print('epoch {} 结束， 用时：{}'.format(epoch,end-start))
        print('centers: {}'.format(centers))

# TEST all 
if __name__ == '__main__':
	assignments,centers,npoints = init_cluster(train_data.data.shape[0],10)
	clustering_train()
	#TODO 
	# coding for presenting result