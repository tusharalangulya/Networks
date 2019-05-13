#include <fstream>
#include "ns3/core-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/config-store-module.h"
#include "ns3/gnuplot.h"
#include "ns3/gnuplot-helper.h"
#include "ns3/netanim-module.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("Assignment4WifiTCP");//this line declares a logging component called FirstScriptExample that allows you to enable and disable console message logging by reference to the name.




int  main (int argc, char *argv[])
 {
   double simulationTime = 5; //in seconds 
   string tcpmode="TcpVegas";//the default tcp variant is taken as TcpVegas but we take the take the variant from command line also
   cout<<"Enter TcpMode";
  cin>>tcpmode;
if(tcpmode=="TcpVegas")
{
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpVegas::GetTypeId ()));  //we are setting the type of congestion control based on the tcp variant we get through user
}
else if(tcpmode=="TcpVeno")
{
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpVeno::GetTypeId ()));
}
else
{
  
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpWestwood::GetTypeId ()));
  Config::SetDefault ("ns3::TcpWestwood::FilterType", EnumValue (TcpWestwood::TUSTIN));
}

// Throughput
 string tplt=tcpmode+"_wired_throughput.plt";
 string tpng=tcpmode+"_wired_throughput.png";
Gnuplot tplot(tpng);
tplot.SetTerminal ("png");
tplot.SetLegend ("PacketSize", "WiredThroughput");
tplot.AppendExtra ("set xrange [30:1600]");
tplot.SetTitle("WiredThroughput vs PacketSize in "+tcpmode);
Gnuplot2dDataset tdata;
tdata.SetTitle ("WiredThroughput");
tdata.SetStyle (Gnuplot2dDataset::LINES_POINTS);

 string fplt=tcpmode+"_wired_fairness.plt";
 string fpng=tcpmode+"_wired_fairness.png";
Gnuplot fplot(fpng);
fplot.SetTerminal ("png");
fplot.SetLegend ("PacketSize", "WiredFairness");
fplot.AppendExtra ("set xrange [30:1600]");
fplot.SetTitle("WiredFairness vs PacketSize  in "+tcpmode);
Gnuplot2dDataset fdata;
fdata.SetTitle ("WiredFairness");
fdata.SetStyle (Gnuplot2dDataset::LINES_POINTS);

  
        
   uint16_t port = 100;

   //uint32_t payloadSize[10] = {1,4,8,12,20,512,536,588,1380,1460};
   uint32_t payloadSize[10] = {40,44,48,52,60,552,576,628,1420,1500}; //various payload sizes of the packets are mentioned and run in a loop
   uint32_t i=0;
for(i=0;i<9;i++){
  NS_LOG_INFO ("Simulation for new packet size.");
 cout<<"\nPacket Size: "<<payloadSize[i]<<endl;
    
  //The maximum segment size (MSS) is the largest amount of data, specified in bytes, that a computer or communications device can handle in a single, unfragmented piece.
   Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (payloadSize[i]));//we set default segment size to the payloadsize
   NS_LOG_INFO ("Node Creation");
    //The first line above just declares a NodeContainer which we call nodes. The second line calls the Create method on the nodes object and asks the container to create two nodes. As described in the Doxygen, the container calls down into the ns-3 system proper to create two Node objects and stores pointers to those objects internally.
   NodeContainer nodes;
   nodes.Create (4);
  

   //Recall that two of our key abstractions are the NetDevice and the Channel. In the real world, these terms correspond roughly to peripheral cards and 
   //network cables. Typically these two things are intimately tied together and one cannot expect to interchange, for example, Ethernet devices and wireless channels
   //. Our Topology Helpers follow this intimate coupling and therefore you will use a single PointToPointHelper to configure
   // and connect ns-3 PointToPointNetDevice and PointToPointChannel objects in this script.
   NS_LOG_INFO ("Establishing Channel.");
   PointToPointHelper p2pnr;    //instantiates a PointToPointHelper object on the stack. for node to router links
   p2pnr.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));//tells to use 100mbps link
   p2pnr.SetChannelAttribute ("Delay", StringValue ("20ms")); //propagation delay
   p2pnr.SetQueue ("ns3::DropTailQueue","Mode",EnumValue (DropTailQueue::QUEUE_MODE_BYTES),"MaxBytes",UintegerValue (250000));

   PointToPointHelper p2prr;  //instantiates a PointToPointHelper object on the stack. for router  to router links
   p2prr.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
   p2prr.SetChannelAttribute ("Delay", StringValue ("50ms"));
   p2prr.SetQueue ("ns3::DropTailQueue","Mode",EnumValue (DropTailQueue::QUEUE_MODE_BYTES),"MaxBytes",UintegerValue (62500));
   

    //For each node in the NodeContainer (there must be exactly two for a point-to-point link) a PointToPointNetDevice is created and saved in the device container. 
   //A PointToPointChannel is created and the two PointToPointNetDevices are attached.
   NetDeviceContainer d1,d2,r1;
   d1 = p2pnr.Install (nodes.Get(0),nodes.Get(1));
   d2=p2pnr.Install (nodes.Get(2),nodes.Get(3));
   r1=p2prr.Install(nodes.Get(1),nodes.Get(2));


   //The InternetStackHelper is a topology helper that is to internet stacks what the PointToPointHelper is to point-to-point net devices.
   // The Install method takes a NodeContainer as a parameter. When it is executed, it will install an Internet Stack (TCP, UDP, IP, etc.) on each of the nodes 
   //in the node container.
   InternetStackHelper stack;
   stack.Install(nodes);
   NS_LOG_INFO("IPV4 address Assigning");

   //declare an address helper object and tell it that it should begin allocating IP addresses from the network 10.3.1.0 using the mask 255.255.255.0 to define 
   //the allocatable bits. By default the addresses allocated will start at one and increase monotonically
   //we have three interfaces
   Ipv4AddressHelper ipv4;
   ipv4.SetBase("10.3.1.0","255.255.255.0");
   //The next line performs the actual address assignment. In ns-3 we make the association between an IP address and a device using an Ipv4Interface object.
   // Just as we sometimes need a list of net devices created by a helper for future reference we sometimes need a list of Ipv4Interface objects.
   // The Ipv4InterfaceContainer provides this functionality.
   Ipv4InterfaceContainer nr1 = ipv4.Assign(d1);

   ipv4.SetBase("10.3.2.0","255.255.255.0");
   Ipv4InterfaceContainer rr = ipv4.Assign(r1);

   ipv4.SetBase("10.3.3.0","255.255.255.0");
   Ipv4InterfaceContainer nr2 = ipv4.Assign(d2);

   NS_LOG_INFO ("Enable static global routing.");
   Ipv4GlobalRoutingHelper::PopulateRoutingTables ();//sets up the routing tables between the nodes
   NS_LOG_INFO("Creating Sink");

   // Create a packet sink to receive these packets
   Address localAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
   PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory",localAddress);
  // GetAny() returns 0.0.0.0 i.e. the listen address for the sink application.
   ApplicationContainer sinkApp = packetSinkHelper.Install (nodes.Get (3));

   NS_LOG_INFO("Creating Client");
   // configures OnOffApplication traffic source to use TCP:
   OnOffHelper onoff ("ns3::TcpSocketFactory",Ipv4Address::GetAny ());
   onoff.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
   onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
   onoff.SetAttribute ("PacketSize", UintegerValue (payloadSize[i]));
   onoff.SetAttribute ("DataRate", StringValue ("100Mbps")); //bit/s


   //start and stop of the apps on devices
   ApplicationContainer apps;
   AddressValue remoteAddress (InetSocketAddress (nr2.GetAddress(1), port));
   onoff.SetAttribute ("Remote", remoteAddress);
   apps.Add (onoff.Install (nodes.Get (0)));

   sinkApp.Start (Seconds (0.0));
   sinkApp.Stop (Seconds (simulationTime + 0.1));
   apps.Start (Seconds (1.0));
   apps.Stop (Seconds (simulationTime + 0.1));



   FlowMonitorHelper flow;
   Ptr<FlowMonitor> monitor = flow.InstallAll();
 
   Simulator::Stop (Seconds (simulationTime + 5));
   Simulator::Run ();
   Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flow.GetClassifier ());
   //monitor->SerializeToXmlFile("try"+to_string(i)+".xml", true, true);
   map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
   int n=0;
   double t=0,t2=0;
  //for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
   //considering only the packets from node(0) to node(1)
   for(map<FlowId, FlowMonitor::FlowStats>::iterator it=stats.begin();it!=stats.end();it++)
   {
    Ipv4FlowClassifier::FiveTuple f = classifier->FindFlow (it->first);
    if(f.sourceAddress!=nr1.GetAddress(0))
    {continue;}
   cout<<"Source Adress: "<<f.sourceAddress<<" Source Port: "<<f.sourcePort<<"\n";
   cout<<"Destination Adress: "<<f.destinationAddress<<" Destination Port: "<<f.destinationPort<<"\n";
   cout << "  Tx Packets:   " << it->second.txPackets << endl;
   cout << "  Tx Bytes:   " << it->second.txBytes << endl;
   cout << "  Rx Packets:   " << it->second.rxPackets << endl;
   cout << "  Rx Bytes:   " << it->second.rxBytes << endl;
   double timetaken= it->second.timeLastRxPacket.GetSeconds() - it->second.timeFirstTxPacket.GetSeconds();
   double Throughput=(it->second.rxBytes*8)/timetaken;
   Throughput/=(1024);
   t+=Throughput;
   t2+=Throughput*Throughput;
   n++;
   cout<<" Throughput :"<<Throughput<<" Kbps\n";
  } 

  //calculation of throughput and Jain's fairness index
 // Raj Jain's equation,
  //  rates the fairness of a set of values where there are n users,  x_{i} is the throughput for the ith connection. The result ranges from 1/n to 1.and it is maximum when all users receive the same allocation. 
  //his index is k/n when  k users equally share the resource, and the other  n-k users receive zero allocation.

  double JainFairnessIndex=t*t/(n*t2);
  cout<<"Jain's Fairness Index: "<<JainFairnessIndex<<"\n";
  double AvgThroughput=t/n;
  cout<<"Average Throughput: "<<AvgThroughput<<"\n";
  fdata.Add(payloadSize[i],JainFairnessIndex);
  tdata.Add(payloadSize[i],AvgThroughput);
   Simulator::Destroy ();
  }
  
  tplot.AddDataset(tdata);
  std :: ofstream tplot1 (tplt.c_str());
  tplot.GenerateOutput (tplot1);
  tplot1.close ();

  fplot.AddDataset(fdata);
  std :: ofstream fplot1 (fplt.c_str());
  fplot.GenerateOutput (fplot1);
  fplot1.close ();
   return 0;
 }
