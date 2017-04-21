#include <bits/stdc++.h>
#include <sys/types.h> 
#include <unistd.h>
#include <stdlib.h>
using namespace std;
#define c1 1.2
int H = 20, C=100;

double random_num(double a, double b)
{	
return a+ rand()%(int)(b-a);
}

class Interval
{
public:
	int left, right, clientID, rangeLeft, rangeRight;
	Interval();
}

Interval::Interval(int left, int right, int clientID, int rangeLeft, int rangeRight)
{
	this->left = left;
	this->right = right;
	this->clientID = clientID;
	this->rangeLeft= rangeLeft;
	this->rangeRight= rangeRight;
}


class Host
{
public:
	double f_residual=random_num(50,150);
	double t_arrival=random_num(0,250);
	double t_departure=random_num(250,1000);
	double host_rating=random_num(80,100)/100;
	double uplink_rate=random_num(40,120), downlink_rate=(80,160);	
	double resourceUtilization;
	vector< Interval > taskSchedule;
	Host();
	double admissionControl(Host *, Client *);
};

Host::Host()
{
	f_residual=random_num(50,150);
	t_arrival=random_num(0,250);
	t_departure=random_num(250,1000);
	host_rating=random_num(80,100)/100;
	uplink_rate=random_num(40,120);
	downlink_rate=random_num(80,160);
	resourceUtilization=0;
	taskSchedule.resize(0);
}

class Client
{
public:
	double task_length;
	double results_length;
	double t_arrival;
	double deadline;
	double t_departure;
	double uplink_rate, downlink_rate;
	Client();	
};

Client::Client()
{
	task_length=random_num(25,100);
	results_length=random_num(50,500);
	t_arrival=random_num(0,1000);
	deadline=random_num(50,1000);
	t_departure=random_num(50,100);
	uplink_rate=random_num(40,120);
	downlink_rate=random_num(80,160);
}

double calculateUtilization(vector<Interval> schedule)
{
	double usedResource=0;
	for(int i=0;i<schedule.size();i++)
		usedResource+=schedule[i].right-schedule[i].left;
	return schedule.size()>0? usedResource/schedule[schedule.size()-1].right : 0;
}

Host::admissionControl(Client *client_list, int client_ID, bool setFlag)
{
	//preprocessing of existing schedule for creating gaps to be filled by new task
	//should use other algorithm which minimizes context switches
	if(taskSchedule.size()==0)
	{
		Client client = client_list[taskSchedule[i].clientID];
		int strt = client.t_arrival + client.task_length/ client.uplink_rate + client.task_length/downlink_rate;
		int endd = min(client.deadline - client.results_length/uplink_rate - client.results_length/client.downlink_rate,
		t_departure - client.results_length/client.downlink_rate);
		if(strt>=endd || endd-strt<client.task_length/f_residual) 
			return calculateUtilization(taskSchedule);
		Interval interval = new Interval(endd - client.task_length/f_residual, endd, client_ID, strt, endd);
		taskSchedule.push_back(interval);
		return calculateUtilization(taskSchedule);
	}

	vector<Interval> rightShifted;
	for(int i=taskSchedule.size()-1;i>=0;i--)
	{
		Client client = client_list[taskSchedule[i].clientID];
		int strt = client.t_arrival + client.task_length/ client.uplink_rate + client.task_length/downlink_rate;
		int endd = client.deadline - client.results_length/uplink_rate - client.results_length/client.downlink_rate; 
		int rightShiftAmount;
		if(i==taskSchedule.size()-1)
		{
			rightShiftAmount = endd - taskSchedule[i].right;
		}
		else
		{
			rightShiftAmount = min(endd, rightShifted[0].left) - taskSchedule[i].right;
		}
		Interval interval = new Interval(taskSchedule[i].left + rightShiftAmount, 
			taskSchedule[i].right + rightShiftAmount,
			taskSchedule[i].clientID,
			taskSchedule[i].t_arrival,
			taskSchedule[i].deadline);
		rightShifted.insert(rightShifted.begin(), interval);		
	}

	//resource allocation for newly admitted task
	Client client = client_list[clientID];
	int strt = client.t_arrival+client.task_length/ client.uplink_rate+client.task_length/downlink_rate;
	int endd = client.deadline-client.results_length/uplink_rate - client.results_length/client.downlink_rate; 
	int gapRight=endd, gapLeft;
	vector<Interval> gapList, newSchedule, updatedSchedule;
	//checking if it can fit or not
	if(endd - client.task_length/f_residual < strt)
	{		
		return calculateUtilization(rightShifted);
	}
	//creates list of gaps where task can be filled
	//check this
	for(int i=rightShifted.size()-1;i>=0;i--)						
	{		
		if(rightShifted[i].right> endd)							
		{
			if(rightShifted[i].left<endd)
				gapRight = rightShifted[i].left;
			else 
				gapRight = endd;
			continue;
		}		`   
		gapLeft = rightShifted[i].right;
		if(gapLeft<strt) 
		{				
			Interval interval = new Interval(endd - client.task_length/f_residual, endd, client_ID, strt, endd);		//check this
			gapList.push_back(interval);
			break;
		}
		while(i>=0 && gapRight>strt)
		{			
			Interval interval=new Interval(max(gapLeft,strt),gapRight,client_ID,strt,endd);
			gapList.push_back(interval);
			gapRight=rightShifted[i].left;
			gapLeft= max(strt, (i>0? rightShifted[i-1].right : 0));
			i--;
		}				
	}
	//fills the gaps
	if(gapList.size()>0)
	{
		gapList.sort();
		int execTimeLeft = client_list[client_ID].task_length/f_residual;
		int i=gapList.size()-1;
		while(i>=0 && execTimeLeft>0)
		{
			if(gapList[i].right - gapList[i].left > execTimeLeft)
			{
				newSchedule.push_back(gapList[i].right - execTimeLeft, gapList[i].right);
				execTimeLeft=0;
			}
			else
			{
				newSchedule.push_back(gapList[i].left, gapList[i].right);
				execTimeLeft - = gapList[i].right - gapList[i].left;
			}
			i--;
		}
		if(execTimeLeft>0) 
		{		
			return calculateUtilization(rightShifted);
		}
		else 
		{
			updatedSchedule = rightShifted;
			updatedSchedule.insert(updatedSchedule.end(), newSchedule.begin(), newSchedule.end());
			updatedSchedule.sort();
			if(setFlag==1) 
			{
				this->taskSchedule = updatedSchedule;
				this->resourceUtilization=calculateUtilization(updatedSchedule);
			}
			return calculateUtilization(updatedSchedule);
		}	
	}
	else 
	{
		return calculateUtilization(rightShifted);
	}
	

}

double calculateMeanUtilization(Host * host_list)
{
	double sum=0;
	for(int i =0;i<H;i++)
		sum+=host_list[i].resourceUtilization;
	return sum/H;
}

void allocateResource(int clientID, Client *client_list, Host host)
{
	host.admissionControl(client_list, clientID, 1);
}


int main()
{
	srand(time(NULL));
	Host host_list[H];
	Client client_list[C];
	double cost_client[C];
	double host_revenue[C][H];
	for(int i=0;i<C;i++)
		cost_client[i]=10+(client_list[i].task_length)/ (client_list[i].deadline - client_list[i].t_arrival);
	for(int i=0;i<C;i++)
	for(int j=0;j<H;j++)		
		host_revenue[i][j]= 5 + host_list[j].host_rating /40.00 * (host_list[j].f_residual/100 + client_list[i].task_length);
	double ETT[C][H], freqs[C][H], tau[C][H], utilities[C][H];
	for(int i=0;i<C;i++)
	{
		double meanUtilization = calculateMeanUtilization(host_list);
		double maxUtility = INT_MIN, bestHost=-1;		
		for(int j=0;j<H;j++)
		{

			resourceUtilization = host_list[j].admissionControl(client_list, i, 0);

			utilities[i][j]=(cost_client[i]-host_revenue[i][j])*host_list[j].host_rating*(1+resourceUtilization - meanUtilization);
			if(utilities[i][j] > maxUtility)
			{
				maxUtility = utilities[i][j];
				bestHost = j;
			}
			// tau[i][j]=client_list[i].task_length/ client_list[i].uplink_rate +client_list[i].results_length/client_list[i].downlink_rate +
			// 		client_list[i].task_length/ host_list[j].downlink_rate +client_list[i].results_length/host_list[j].uplink_rate + 1;
			// freqs[i][j]=min( client_list[i].deadline , host_list[j].t_departure) - 1 - tau[i][j]>0? 
			// (1 * client_list[i].task_length)/(min( client_list[i].deadline , host_list[j].t_departure) - 1 - tau[i][j])
			// : 0.0001;
			// ETT[i][j]=(client_list[i].task_length / freqs[i][j]) + tau[i][j];		
			// utilities[i][j] = (cost_client[i]-host_revenue[i][j])*host_list[j].host_rating*(client_list[i].deadline - ETT[i][j]);
			// cout<<cost_client[i]<<" "<<host_revenue[i][j]<<"\n";
		}
		allocateResource(client_list[i], bestHost);

	}
	return 0;
}
