#include <bits/stdc++.h>
#include <sys/types.h> 
#include <unistd.h>
#include <stdlib.h>
using namespace std;

int clientCounter=0, hostCounter=0;
double meanFRes=100.00;
double epochTime=100.00;
double meanDataRate=meanFRes*100;
double targetSystemLoad=1.00;

double random_num(double a, double b)
{	
	return a+ rand()%(int)(b-a);
}

struct Interval
{
	double left, right, rangeLeft, rangeRight;
	int clientID;
	Interval(){}
	Interval(double, double, int, double, double);
	bool operator < (const Interval& intrvl) const
	{
		return (left<intrvl.left)||(left==intrvl.left && right<intrvl.right);
	}
};

Interval::Interval(double left, double right, int clientID, double rangeLeft, double rangeRight)
{
	this->left = left;
	this->right = right;
	this->clientID = clientID;
	this->rangeLeft= rangeLeft;
	this->rangeRight= rangeRight;
}

class Client
{
public:
	bool isScheduled;
	int client_ID;
	double task_length;
	double results_length;
	double t_arrival;
	double deadline;
	double completionTime;
	double uplink_rate, downlink_rate;
	Client();	
};

Client::Client()
{
	isScheduled=false;
	client_ID=clientCounter++;
	task_length= meanFRes * random_num(80,120);				//bits
	results_length=meanFRes * random_num(80,120);				//bits
	t_arrival=random_num(80,120);						//seconds
	deadline=t_arrival+100.00*(task_length/meanFRes)/random_num(10,20);	//seconds	
	uplink_rate=meanFRes * random_num(80,120)/1.00;				//bits per second
	downlink_rate=meanFRes * random_num(80,120)/2.00;			//bits per second
	completionTime=0.00;
}

class Host
{
public:
	int host_ID;
	double f_residual;			//bits per second
	double t_arrival;						////seconds
	double t_departure;			//seconds
	double host_rating;
	double uplink_rate, downlink_rate;	//bits per second
	double resourceUtilization;
	vector< Interval > taskSchedule;
	Host();
	pair<bool, double> admissionControl(vector<Client> &, int, bool);
	void allocateResource(int, vector<Client> &);
};

Host::Host()
{
	host_ID=hostCounter++;
	f_residual= meanFRes * random_num(8,12) / 10.00;
	t_arrival=random_num(80,500);						//seconds
	t_departure=random_num(6000,8000);
	//host_rating=random_num(80,100)/100;
	host_rating=1.00;
	uplink_rate=meanFRes * random_num(80,120)/1.00;			//bits per second
	downlink_rate=meanFRes * random_num(80,120)/2.00;			//bits per second
	resourceUtilization=0.00;
	taskSchedule.resize(0);
}

double calculateUtilization(vector<Interval> schedule, double t_dep)
{
	double usedResource=0.00;
	for(int i=0;i<schedule.size();i++)
		usedResource+= schedule[i].right-schedule[i].left;
	return schedule.size()>0 ? usedResource/t_dep: 0.00; //schedule[schedule.size()-1].right : 0.00;
}

double calculateUtilizationTillNextIteration(vector<Interval> schedule, double t_dep)
{
	double usedResource=0.00;
	for(int i=0;i<schedule.size();i++)
	{
		if(schedule[i].right>= epochTime &&  schedule[i].left>=epochTime)
			break;
		else if(schedule[i].right>= epochTime &&  schedule[i].left<=epochTime)
		{
			usedResource+=epochTime - schedule[i].left;
			break;
		}
		else
		usedResource+= schedule[i].right-schedule[i].left;
	}
	return usedResource/epochTime;
}
pair<bool,double> Host::admissionControl(vector<Client> &client_list, int client_ID, bool setFlag)
{
	//preprocessing of existing schedule for creating gaps to be filled by new task
	//should use other algorithm which minimizes context switches
	if(taskSchedule.size()==0)
	{
		int cPos=-1;					//changeD this - find the client with client_ID - was clientIndex
		for(int i=0;i<client_list.size();i++)
		if(client_list[i].client_ID == client_ID)
		{
			cPos=i;			
			break;
		}
		Client client = client_list[cPos];
		double strt = max(t_arrival + client.task_length/ downlink_rate,
			client.t_arrival + client.task_length/ client.uplink_rate + client.task_length/downlink_rate);
		double endd = min(client.deadline - client.results_length/uplink_rate - client.results_length/client.downlink_rate,
		t_departure - client.results_length/client.downlink_rate);
		if(strt>=endd || endd-strt<client.task_length/f_residual) 
			return make_pair(false,calculateUtilization(taskSchedule, t_departure));
		Interval interval = Interval(endd - client.task_length/f_residual, endd, client_ID, strt, endd);
		if(setFlag==true)
		{
			taskSchedule.push_back(interval);
			client_list[cPos].completionTime=endd;
			resourceUtilization=calculateUtilization(taskSchedule, t_departure);
		}
		return make_pair(true,calculateUtilization(taskSchedule, t_departure));
	}

	vector<Interval> rightShifted;
	for(int i=taskSchedule.size()-1;i>=0;i--)
	{		
		double strt = max(taskSchedule[i].rangeLeft, t_arrival);
		double endd = min(taskSchedule[i].rangeRight, t_departure);
		double rightShiftAmount;
		if(i==taskSchedule.size()-1)
		{
			rightShiftAmount = endd - taskSchedule[i].right;
		}
		else
		{
			rightShiftAmount = min(endd, rightShifted[0].left) - taskSchedule[i].right;
		}
		Interval interval = Interval(taskSchedule[i].left + rightShiftAmount, 
			taskSchedule[i].right + rightShiftAmount,
			taskSchedule[i].clientID,
			strt,
			endd);
		if(interval.left< interval.right)
			rightShifted.insert(rightShifted.begin(), interval);	
	}

	//resource allocation for newly admitted task
	int cPos=-1;					//changeD this - find the client with client_ID - was clientIndex
	for(int i=0;i<client_list.size();i++)
	if(client_list[i].client_ID == client_ID)
	{
		cPos=i;			
		break;
	}
	Client client = client_list[cPos];
	double strt = max(t_arrival + client.task_length/ downlink_rate,
			client.t_arrival + client.task_length/ client.uplink_rate + client.task_length/downlink_rate);
	double endd = min(client.deadline - client.results_length/uplink_rate - client.results_length/client.downlink_rate,
			t_departure - client.results_length/client.downlink_rate);
	double gapRight=endd, gapLeft=strt;
	vector<Interval> gapList, newSchedule, updatedSchedule;
	//checking if it can fit or not
	if(endd - client.task_length/f_residual <= strt)
	{		
		return make_pair(false, calculateUtilization(rightShifted, t_departure));
	}
	//creates list of gaps where task can be filled
	//checked this
	int i = rightShifted.size()-1;
	bool flag=0;
	while(i>=0 && rightShifted[i].right>= endd && rightShifted[i].left>=endd)							
	{
		i--;
		if(i==-1)
		{
			gapLeft=strt;
			Interval interval = Interval(max(gapLeft,strt), gapRight, client_ID, strt, endd);		//check this
			gapList.push_back(interval);
		}
	} 
	if(i>=0 && rightShifted[i].right>=endd && rightShifted[i].left<endd)
	{	
		gapRight=rightShifted[i].left;
		gapLeft= max(strt, (i>0? rightShifted[i-1].right : 0));
		i--;
		flag=1;
	}
	else if(i>=0 && rightShifted[i].right <endd && rightShifted[i].right<=strt)
	{
		gapLeft=strt;
		Interval interval = Interval(max(gapLeft,strt), gapRight, client_ID, strt, endd);		//check this
		gapList.push_back(interval);
		flag=0;
	}
	else if(i>=0 && rightShifted[i].right< endd && rightShifted[i].right>strt)
	{
		gapLeft=rightShifted[i].right;
		Interval interval = Interval(max(gapLeft,strt), gapRight, client_ID, strt, endd);		//check this	
		gapList.push_back(interval);
		gapRight=rightShifted[i].left;
		gapLeft= max(strt, (i>0? rightShifted[i-1].right : 0));
		if(i==0 && gapRight>max(gapLeft,strt))
		{
			Interval interval = Interval(max(gapLeft,strt),gapRight,client_ID,strt,endd);		
			if(interval.left<interval.right) 
				gapList.push_back(interval);
		}
		i--;
		flag=1;
	}
	while(flag==true && i>=0 && gapRight>strt)
	{	
		Interval interval = Interval(max(gapLeft,strt),gapRight,client_ID,strt,endd);
		if(interval.left<interval.right) 
			gapList.push_back(interval);
		gapRight=rightShifted[i].left;
		gapLeft= max(strt, (i>0? rightShifted[i-1].right : 0));
		if(i==0 && gapRight>max(gapLeft,strt))
		{
			Interval interval = Interval(max(gapLeft,strt),gapRight,client_ID,strt,endd);		
			if(interval.left<interval.right) 
				gapList.push_back(interval);
		}
		i--;
	}	
	// for(int i=rightShifted.size()-1;i>=0;i--)						
	// {		
	// 	if(rightShifted[i].right> endd)							
	// 	{
	// 		if(rightShifted[i].left<endd)
	// 			gapRight = rightShifted[i].left;
	// 		else 
	// 			gapRight = endd;
	// 		continue;
	// 	}   
	// 	gapLeft = rightShifted[i].right;
	// 	if(gapLeft<strt) 
	// 	{				
	// 		Interval interval = Interval(max(gapLeft,strt), gapRight, client_ID, strt, endd);		//check this
	// 		gapList.push_back(interval);
	// 		break;
	// 	}
	// 	while(i>=0 && gapRight>strt)
	// 	{			
	// 		Interval interval = Interval(max(gapLeft,strt),gapRight,client_ID,strt,endd);
	// 		if(interval.left<interval.right) 
	// 			gapList.push_back(interval);
	// 		gapRight=rightShifted[i].left;
	// 		gapLeft= max(strt, (i>0? rightShifted[i-1].right : 0));
	// 		i--;
	// 	}				
	// }
	//fills the gaps
	if(gapList.size()>0)
	{
		sort(gapList.begin(), gapList.end());	
		// cout<<"\nGAPLIST:\n";
		// for(int i=0;i<gapList.size();i++)
		// cout<<gapList[i].left<<" "<<gapList[i].right<<endl;	
		int cPos=-1;
		for(int i=0;i<client_list.size();i++)
		if(client_list[i].client_ID == client_ID)
		{
			cPos=i;			
			break;
		}
		Client client = client_list[cPos];

		double execTimeLeft = client.task_length/f_residual;
		int j=gapList.size()-1;
		while(j>=0 && execTimeLeft>0.00)
		{
			if(gapList[j].right - gapList[j].left >= execTimeLeft)
			{
				Interval interval = Interval(gapList[j].right - execTimeLeft, gapList[j].right, client_ID, strt, endd);
				newSchedule.push_back(interval);
				execTimeLeft=0.00;
			}
			else
			{
				Interval interval = Interval(gapList[j].left, gapList[j].right, client_ID, strt, endd);
				newSchedule.push_back(interval);
				execTimeLeft-= gapList[j].right - gapList[j].left;
			}
			j--;
		}
		if(execTimeLeft>0.00) 
		{		
			return make_pair(false,calculateUtilization(rightShifted, t_departure));
		}
		else 
		{
			updatedSchedule.resize(0);
			updatedSchedule = rightShifted;
			if(setFlag==true)
			{
				sort(newSchedule.begin(), newSchedule.end());
				client_list[cPos].completionTime = newSchedule[newSchedule.size()-1].right;
			}
			for(int i=0;i<newSchedule.size();i++)
				updatedSchedule.push_back(newSchedule[i]);			
			sort(updatedSchedule.begin(), updatedSchedule.end());
			if(setFlag==true) 
			{
				//cout<<"\nhi\n";
				taskSchedule.resize(0);
				for(int j=0;j<updatedSchedule.size();j++)
					taskSchedule.push_back(updatedSchedule[j]);
				resourceUtilization=calculateUtilization(taskSchedule, t_departure);
			}
			return make_pair(true,calculateUtilization(updatedSchedule, t_departure));
		}	
	}
	else 
	{
		return make_pair(false, calculateUtilization(rightShifted, t_departure));
	}
}

double calculateMeanUtilization(vector<Host> host_list)
{
	double sum=0;
	for(int i =0;i<host_list.size();i++)
		sum+=host_list[i].resourceUtilization;
	return host_list.size()>0 ? sum/host_list.size() : 0.00;
}

void Host::allocateResource(int clientID, vector<Client> &client_list)
{
	if(admissionControl(client_list, clientID, true).first == false) cout<<"\nSOMETHING IS VERY WRONG\n";
}

//may or may not consider switching of tasks
//may or may not consider minimization of context switches

double calculateFairness(vector<Host> host_list)
{
	double fairness=0.00, sum=0.00, denom=0.00;
	for(int i=0;i<host_list.size();i++)
	{
		double util = calculateUtilization(host_list[i].taskSchedule, host_list[i].t_departure);
		sum+=util;
		denom+=util*util;
	}
	denom *=host_list.size();
	sum *= sum;
	return denom==0.00? 1.00:sum/denom;
}

int main()
{
	int iterations = 0;
	vector<Host> host_list;
	vector<Client> client_list;
	srand(time(NULL));
	ofstream resUtil, unscheduleds, datapoint, fairness;
	resUtil.open("meanResourceUtilization_iterations.dat");
	unscheduleds.open("unscheduledTasks_iterations.dat");
	datapoint.open("datapoints.dat", ios_base::app);
	fairness.open("fairness.dat");
	double totalProfit=0;
	int unscheduledTasks=0;
	double totalUtilization=0.00, averageUtilization = 0.00;
	double totalSystemLoad=0.00, meanSystemLoad=0.00;
	double totalResUtil=0.00, totalFairness=0.00;
	int h,c;
	int targetIterations=0;
	vector<Client> scheduledTasks; 	
	while(iterations++ < 80)
	{	
		if(iterations==1)
		{
			h=8;
			c=2;
		}
		else 
		{
			h=0;
			c=0;
		}
		//random_num(8,10);	
		vector<Host> new_hosts, new_host_list;
		vector<Client> new_clients, new_client_list;
		new_hosts.resize(0);
		new_clients.resize(0);
		new_host_list.resize(0);
		new_client_list.resize(0);

		//calculation of resUtil and system load in previous iteration
		double systemLoad=0.00;			
		double usedResource=0.00, availableResource=0.00;		
		int C= client_list.size();
		int H= host_list.size();
		double sumCapacity=0.00;
		//totalUtilization+= calculateMeanUtilization(host_list);
		for(int i=0;i<H;i++)
		{
			usedResource+= calculateUtilizationTillNextIteration(host_list[i].taskSchedule, host_list[i].t_departure) * host_list[i].f_residual;
			availableResource+= host_list[i].f_residual;
			sumCapacity+= (6.00) * (host_list[i].f_residual/meanFRes); //- calculateUtilizationTillNextIteration(host_list[i].taskSchedule, host_list[i].t_departure)
		}
		totalUtilization=availableResource>0.00 ? usedResource/availableResource : 0;
		averageUtilization=totalUtilization;
		//averageUtilization= iterations==1? 0.00 : totalUtilization/(iterations-1);
		double sumWeight=0.00;
		for(int i=0;i<C;i++)
		{				
			if(client_list[i].isScheduled==false || (client_list[i].isScheduled==true && client_list[i].completionTime>0.00) )
			sumWeight+= 1.00*(client_list[i].task_length/meanFRes)/ (client_list[i].deadline - client_list[i].t_arrival);// -2*client_list[i].task_length/meanDataRate);
		}
		for(int i=0;i<scheduledTasks.size();i++)
		{
			sumWeight+= 1.00* (scheduledTasks[i].task_length/meanFRes)/ (scheduledTasks[i].deadline - scheduledTasks[i].t_arrival);// -2*client_list[i].task_length/meanDataRate);
		}
		systemLoad = sumCapacity > 0.00 ? (sumWeight/sumCapacity) : 0.00;
		//cout<<endl<<C<<" "<<H<<endl;
		//cout<<endl<<sumWeight<<" "<<sumCapacity<<endl;
		if(iterations!=1)
		{
			if(systemLoad<targetSystemLoad-0.05)
			{
				c = (targetSystemLoad*sumCapacity - sumWeight)*.80;
				//cout<<endl<<c<<" clients created\n";
				h = 0;
			}
			else if(systemLoad>targetSystemLoad+0.05)
			{
				c = 0;
				h = (sumWeight/targetSystemLoad- sumWeight/systemLoad)/140.00;
				//cout<<endl<<h<<" hosts created\n";
			}
		}
		//creation of new hosts and clients
		for(int i=0;i<h;i++)
			new_hosts.push_back(Host());
		for(int i=0;i<c;i++)
			new_clients.push_back(Client());

		//time correction in pre-existing hosts and clients
		if(host_list.size()>0)
		{
			//compare systemload with desired value and create hosts if it is more, create clients if less			
			for(int i=0;i<host_list.size();i++)
			{				
				host_list[i].t_arrival=max(0.00,host_list[i].t_arrival- epochTime );
				host_list[i].t_departure=max(0.00,host_list[i].t_departure- epochTime);
				if(host_list[i].t_departure==0.00) continue;				
				//correction of taskSchedule
				vector<Interval> newTaskSchedule;
				newTaskSchedule.resize(0);

				for(int j=0;j<host_list[i].taskSchedule.size();j++)
				{
					Interval interval = host_list[i].taskSchedule[j];
					interval.left = max(0.00, interval.left-epochTime);
					interval.right= max(0.00, interval.right-epochTime);
					interval.rangeLeft=max(0.00, interval.rangeLeft-epochTime);
					interval.rangeRight=max(0.00, interval.rangeRight-epochTime);
					if(interval.right==0.00) continue;
					newTaskSchedule.push_back(interval);
				}

				host_list[i].taskSchedule=newTaskSchedule;
				host_list[i].resourceUtilization=calculateUtilization(newTaskSchedule, host_list[i].t_departure);
				new_host_list.push_back(host_list[i]);
			}
			host_list.resize(0);
			for(int i=0;i<new_host_list.size();i++)
				host_list.push_back(new_host_list[i]);
			for(int i=0;i<new_hosts.size();i++)
				host_list.push_back(new_hosts[i]);
		}		
		else 
		{
			for(int i=0;i<new_hosts.size();i++)
				host_list.push_back(new_hosts[i]);
		}
		
		if(client_list.size()>0)
		{
			for(int i=0;i<client_list.size();i++)
			{
				if(client_list[i].isScheduled==false) 
				{				
					client_list[i].t_arrival=max(0.00,client_list[i].t_arrival-epochTime);					
					client_list[i].deadline=max(0.00,client_list[i].deadline-epochTime);
					if(client_list[i].deadline==0.00) 
					{
						unscheduledTasks++;						
						continue;				
					}
					new_client_list.push_back(client_list[i]);
				}
				else
				{
					//cout<<"\nadding to scheduledTasks"<<endl;
					scheduledTasks.push_back(client_list[i]);
				}
			}
			client_list.resize(0);
			for(int i=0;i<new_client_list.size();i++)
				client_list.push_back(new_client_list[i]);
			for(int i=0;i<new_clients.size();i++)
				client_list.push_back(new_clients[i]);
		}
		else 
		{			
			for(int i=0;i<new_clients.size();i++)
				client_list.push_back(new_clients[i]);
		}

		if(scheduledTasks.size()>0)
		{			
			vector<Client> newScheduledTasks;
			newScheduledTasks.resize(0);
			for(int i=0;i<scheduledTasks.size();i++)
			{
				scheduledTasks[i].t_arrival=max(0.00,scheduledTasks[i].t_arrival-epochTime);					
				scheduledTasks[i].deadline=max(0.00,scheduledTasks[i].deadline-epochTime);
				scheduledTasks[i].completionTime=scheduledTasks[i].completionTime-epochTime;
				if(scheduledTasks[i].completionTime>0.00)
					newScheduledTasks.push_back(scheduledTasks[i]);		
				// else
				// 	cout<< scheduledTasks[i].client_ID<<" Task Completed at"<<scheduledTasks[i].completionTime<<endl;	
			}
			scheduledTasks.resize(0);
			for(int i=0;i<newScheduledTasks.size();i++)
				scheduledTasks.push_back(newScheduledTasks[i]);
		}
		// cout<<"\nscheduled client_list at iteration "<< iterations <<" is:\n\n";
		// cout<<"client_ID\t ArrivalTime\tDeadline\tTaskLength\tResultLength\tCompletionTime\n";
		
		// for(int i=0;i<scheduledTasks.size();i++)
		// cout<<scheduledTasks[i].client_ID<<"\t\t"<<scheduledTasks[i].t_arrival<<"\t\t"<<scheduledTasks[i].deadline<<"\t\t"<<scheduledTasks[i].task_length<<"\t\t"<<scheduledTasks[i].results_length<<"\t"<<scheduledTasks[i].completionTime - epochTime<<"\n";
		// double usedResource=0.00, availableResource=0.00;		
		C= client_list.size();
		H= host_list.size();
		
		//sorting the active client list
		vector< pair< double, int > > cost_index;
		for(int i=0;i<C;i++)
			cost_index.push_back(make_pair(4.00 + 1/500.00 * (client_list[i].task_length)/ (client_list[i].deadline - client_list[i].t_arrival), i) );
		//10.00 + 1/500.00 * (client_list[i].task_length)/ (client_list[i].deadline - client_list[i].t_arrival)
		sort(cost_index.begin(), cost_index.end());
		vector<Client> sorted_client_list;
		sorted_client_list.resize(0);
		for(int i=cost_index.size()-1;i>=0;i--)							//show graphs for observations after reversing this loop
			sorted_client_list.push_back(client_list[cost_index[i].second]);
		client_list.resize(sorted_client_list.size());
		for(int i=0;i<client_list.size();i++)
			client_list[i]=sorted_client_list[i];			
		
		// cout<<client_list[i].client_ID<<"\t\t"<<client_list[i].t_arrival<<"\t\t"<<client_list[i].deadline<<"\t\t"<<client_list[i].task_length<<"\t\t"<<client_list[i].results_length<<"\t"<<client_list[i].completionTime - epochTime<<"\n";

		cout<<"\n\n====================================================================================================================================\n\n";
		cout<<"\nactive client_list at iteration "<< iterations <<" is:\n\n";
		cout<<"client_ID\t ArrivalTime\tDeadline\tTaskLength\tResultLength\tCompletionTime\n";
		for(int i=0;i<C;i++)
		cout<<client_list[i].client_ID<<"\t\t"<<client_list[i].t_arrival<<"\t\t"<<client_list[i].deadline<<"\t\t"<<client_list[i].task_length<<"\t\t"<<client_list[i].results_length<<"\t"<<client_list[i].completionTime - epochTime<<"\n";
		cout<<"\nactive host_list at iteration "<< iterations <<" is:\n\n";
		cout<<"host_ID\t ArrivalTime\tDepartureTime\tResidualFrequency\tCPUUtilization\n";
		for(int i=0;i<H;i++)
		cout<<host_list[i].host_ID<<"\t\t"<<host_list[i].t_arrival<<"\t\t"<<host_list[i].t_departure<<"\t\t"<<host_list[i].f_residual<<"\t\t"<<host_list[i].resourceUtilization<<"\n";

		double cost_client[C];
		double host_revenue[C][H];
		for(int i=0;i<C;i++)
			cost_client[i]=4.10 + 1/500.00 * (client_list[i].task_length)/ (client_list[i].deadline - client_list[i].t_arrival);
		for(int i=0;i<C;i++)
		for(int j=0;j<H;j++)		
			host_revenue[i][j]= (host_list[j].host_rating /50.00) * (host_list[j].f_residual/10 + client_list[i].task_length/1000);
		//2 + (host_list[j].host_rating /50.00) * (host_list[j].f_residual/10 + client_list[i].task_length/1000);
		double utilities[C][H];
		double profit=0;
		for(int i=0;i<C;i++)
		{
			double meanUtilization = calculateMeanUtilization(host_list);
			double maxUtility = 0, resourceUtilization;
			int bestHost=-1;		
			for(int j=0;j<H;j++)
			{
				pair< bool, double > imaginarySchedule = host_list[j].admissionControl(client_list, client_list[i].client_ID, false);
				resourceUtilization = imaginarySchedule.second;
				
				utilities[i][j]=(cost_client[i]-host_revenue[i][j])
				*host_list[j].host_rating
				/(resourceUtilization==meanUtilization? 0.0001 : max(0.0001,abs(resourceUtilization - meanUtilization))); //can compare 3 different utilities
				
				//cout<<"\nClient "<<client_list[i].client_ID<<" on Host "<<host_list[j].host_ID;
				
				if(imaginarySchedule.first==false);
					//cout<<" | NOT POSSIBLE\n";					
				else 
				{
					//cout<<" | POSSIBLE | PROFIT IS "<<(cost_client[i]-host_revenue[i][j])<<" | UTILITY IS "<<utilities[i][j]<<"\n";	
					
					if(utilities[i][j] >= maxUtility)
					{					
						maxUtility = utilities[i][j];
						bestHost = j;						
					}	
				}													
			}
			//running average proportional fairness
			//for measuring: jain's fairness index
			if(bestHost!=-1)
			{
				//cout<<"ALLOCATED"<<endl;
				host_list[bestHost].allocateResource(client_list[i].client_ID, client_list);
				//cout<<"--------------------------------------\n";
				//cout<<"Task of client "<<client_list[i].client_ID<<" allocated to host "<<host_list[bestHost].host_ID<<endl;
				client_list[i].isScheduled=true;
				// cout<<"\nNew Schedule of host "<<host_list[bestHost].host_ID<<" is:\n";
				//  for(int k=0;k<host_list[bestHost].taskSchedule.size();k++)
				//  	cout<<host_list[bestHost].taskSchedule[k].left<<" to "<<host_list[bestHost].taskSchedule[k].right<<" : " << "task of client "<<host_list[bestHost].taskSchedule[k].clientID<<endl;
				// cout<<"--------------------------------------\n";
				profit+=cost_client[i]-host_revenue[i][bestHost];
			}
		}
		totalProfit+=profit;
		//resUtil<<iterations<<"\t"<<calculateMeanUtilization(host_list)<<endl;	
		//resUtil<<systemLoad<<"\t"<<100.00*averageUtilization<<endl;	
		resUtil<<100.00*systemLoad<<"\t"<<100.00*averageUtilization<<endl;		
		//unscheduleds<<100.00*systemLoad<<"\t"<<unscheduledTasks<<endl;
		totalFairness += calculateFairness(host_list);
		if(iterations>10.00 && systemLoad>=targetSystemLoad-0.3 && systemLoad<=targetSystemLoad +0.3 )
		{
			totalSystemLoad+=systemLoad;
			totalResUtil+=100.00*averageUtilization;
			targetIterations++;
		}
	}
	fairness<<100.00*totalFairness/iterations<<endl;
	unscheduleds<<100.00*unscheduledTasks /clientCounter<<endl; 
	datapoint<<100.00* totalSystemLoad/targetIterations<<" "<<totalResUtil/targetIterations <<endl;
	datapoint.close();
	resUtil.close();
	fairness.close();
	unscheduleds.close();
	return 0;
}
