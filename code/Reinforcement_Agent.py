import torch
import torch.nn.functional as f


class DQN(torch.nn.Module):
    def __init__(self,n_state):
        #self.writter = logger #Tensorboard Loggger
        super(DQN, self).__init__()
        self.model = torch.nn.Sequential(
            torch.nn.Linear(n_state,256),
            torch.nn.ReLU(),
            torch.nn.Linear(256,64),
            torch.nn.ReLU(),
            torch.nn.Linear(64, 16),
            torch.nn.ReLU(),
            torch.nn.Linear(16,2),
            torch.nn.ReLU(),
        )
    def forward(self,x):
        return self.model(x)



# 定义A2C模型
class A2C(torch.nn.Module):
    def __init__(self,n_state):
        super(A2C,self).__init__()
        self.ln1 = torch.nn.Linear(n_state,128)
        self.ln2 = torch.nn.Linear(128,32)
        self.policy = torch.nn.Linear(32,2)
        self.svalue = torch.nn.Linear(32,1)
    def forward(self,x):
        x = F.relu(self.ln1(x))
        x = F.relu(self.ln2(x))
        probs = F.softmax(self.policy(x),dim=-1)
        vvalue = self.svalue(x)
        return probs,vvalue

