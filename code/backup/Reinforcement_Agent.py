import torch



class DQN(torch.nn.Module):
    def __init__(self,n_state):
        #self.writter = logger #Tensorboard Loggger
        self.model = torch.nn.Sequential(
            torch.nn.Linear(n_state,128),
            torch.nn.ReLU(),
            torch.nn.Linear(128,32),
            torch.nn.ReLU(),
            torch.nn.Linear(32,2),
        )
    
    def forward(self,x):
    	return self.model(x)
        


# 定义A2C模型
class A2C(torch.nn.Module):
    def __init__(self,n_state):
        super(A2C_Model,self).__init__()
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

