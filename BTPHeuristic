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
}

class Host
{
public:
	double f_residual=random_num(50,150);
	double t_arrival=random_num(0,250);
	double t_departure=random_num(250,1000);
	double host_rating=random_num(80,100)/100;
	double uplink_rate=random_num(40,120), downlink_rate=(80,160);	
	vector< Interval > taskSchedule;
	Host();
	void admissionControl(Host *, Client *);
};

Host::Host()
{
	f_residual=random_num(50,150);
	t_arrival=random_num(0,250);
	t_departure=random_num(250,1000);
	host_rating=random_num(80,100)/100;
	uplink_rate=random_num(40,120);
	downlink_rate=random_num(80,160);
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

Host::admissionControl(Client *client_list, int client_ID)
{
	double utility;

	//should use other algorithm which minimizes context switches
	vector<Interval> rightShifted;
	for(int i=taskSchedule.size()-1;i>=0;i--)
	{
		Client client = client_list[taskSchedule[i].clientID];
		Interval interval;
		int rightShiftAmount = i==taskSchedule.size()-1? client.deadline - taskSchedule[i].right : min(client.deadline,  rightShifted[0].left) - taskSchedule[i].right; //consider communication time later.
		interval.right = taskSchedule[i].right + rightShiftAmount; 
		interval.left = taskSchedule[i].left + rightShiftAmount;
		interval.clientID = taskSchedule[i].clientID;
		interval.rangeRight = taskSchedule[i].deadline;
		interval.rangeLeft = taskSchedule[i].t_arrival;
		rightShifted.insert(rightShifted.begin(), interval);		
	}

	//resource allocation for newly admitted task
	int strt = client_list[client_ID].t_arrival, endd = client_list[client_ID].deadline, ; //consider communication time
	int gapRight, gapLeft;
	vector<Interval> temp;
	for(int i=rightShifted.size()-1;i>=0;i--)
	{		
		if(rightShifted[i].right> endd) continue;		
		gapRight = endd;
		gapLeft = rightShifted[i].right;
		if(gapLeft<strt) 
		{
			Interval interval;
			interval.left= end - client_list[client_ID].task_length/f_residual;
			if(interval.left < strt) {utility = -1; return;}
			interval.right=endd;
			interval.clientID=client_ID;
			interval.rangeLeft=strt;		//consider communication time
			interval.rangeRight=endd;		//consider communication time
			temp.push_back(interval);
			utility=1;				//edit this
			return;
		}
		while(i>=0 && gapLeft>strt)
		{			
			if(gapLeft<gapRight)
			{
				Interval interval;
				interval.left=max(gapLeft,strt);
				interval.right=gapRight;
				interval.clientID=client_ID;
				interval.rangeLeft=strt;		//consider communication time
				interval.rangeRight=endd;		//consider communication time
				temp.push_back(interval);
			}
			gapRight=rightShifted[i].left;
			gapLeft= max(strt, (i>0? rightShifted[i-1].right : 0));
			i--;
		}
		if(gapLeft==strt && gapLeft<gapRight)
		{
			Interval interval;
			interval.left=max(gapLeft,strt);
			interval.right=gapRight;
			interval.clientID=client_ID;
			interval.rangeLeft=strt;		//consider communication time
			interval.rangeRight=endd;		//consider communication time
			temp.push_back(interval);
		}
		

		
	}
	if(flag==-1)
	{

	}
	else
	{
		int i=flag;
		while(i<rightShifted.size() && rightShifted[i].left > client_list[clientID].t_arrival && rightShifted[i].right < client_list[clientID].deadline)
		{
			
			i++;
		}
	}

}

//function for schedule update after every iteration

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
	for(int j=0;j<H;j++)
	{

		// tau[i][j]=client_list[i].task_length/ client_list[i].uplink_rate +client_list[i].results_length/client_list[i].downlink_rate +
		// 		client_list[i].task_length/ host_list[j].downlink_rate +client_list[i].results_length/host_list[j].uplink_rate + 1;
		// freqs[i][j]=min( client_list[i].deadline , host_list[j].t_departure) - 1 - tau[i][j]>0? 
		// (1 * client_list[i].task_length)/(min( client_list[i].deadline , host_list[j].t_departure) - 1 - tau[i][j])
		// : 0.0001;
		// ETT[i][j]=(client_list[i].task_length / freqs[i][j]) + tau[i][j];		
		// utilities[i][j] = (cost_client[i]-host_revenue[i][j])*host_list[j].host_rating*(client_list[i].deadline - ETT[i][j]);
		// cout<<cost_client[i]<<" "<<host_revenue[i][j]<<"\n";
	}
	ofstream outfile;
	outfile.open("ilp.lp");
	outfile<< "max: ";
	for(int i=0;i<C;i++)
	for(int j=0;j<H;j++)
	{
		outfile<<utilities[i][j]<<" x"<<i<<"_"<<j<<" +" ;
	}
	outfile<<"0;\n\n";
	for(int i=0;i<C;i++)
	{
		for(int j=0;j<H;j++)
		{
			outfile<<"x"<<i<<"_"<<j<<" + ";
		}
		outfile<<"0 <= 1;\n";
	}

	for(int	j=0;j<H;j++)
	{
		for(int i=0;i<C;i++)
		{
			outfile<< freqs[i][j] <<" x"<<i<<"_"<<j<<" + ";
		}
		outfile<<"0 <= "<< host_list[j].f_residual<<";\n";
	}
	for(int i=0;i<C;i++)
	for(int j=0;j<H;j++)
	{
		outfile<<"int x"<<i<<"_"<<j<<";\n";
	}
	outfile.close();
	system("lp_solve ilp.lp > arnav.txt");
	ifstream infile;
	infile.open("arnav.txt");
	return 0;
}
