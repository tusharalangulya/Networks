#include <fstream>
#include "ns3/core-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/config-store-module.h"
#include "ns3/gnuplot.h"
#include "ns3/gnuplot-helper.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"
#include "ns3/nstime.h"
#include "ns3/propagation-delay-model.h"
#include <string.h>

NS_LOG_COMPONENT_DEFINE ("wifi-tcp");//this line declares a logging component called FirstScriptExample that allows you to enable and disable console message logging by reference to the name.

using namespace ns3;
using namespace std;
int main()
{
    double simulationTime = 5; //seconds
    string tcpmode="TcpVegas";////the default tcp variant is taken as TcpVegas but we take the take the variant from command line also
    cout<<"Enter TcpMode";
   cin>>tcpmode;
 if(tcpmode=="TcpVegas")
 {
   Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpVegas::GetTypeId ()));//we are setting the type of congestion control based on the tcp variant we get through user
 }
 else if(tcpmode=="TcpVeno")
 {
   Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpVeno::GetTypeId ()));
 }
 else
 {
   // the default protocol type in ns3::TcpWestwood is WESTWOOD
   Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpWestwood::GetTypeId ()));
   Config::SetDefault ("ns3::TcpWestwood::FilterType", EnumValue (TcpWestwood::TUSTIN));
 }

 //Fragmentation Threshold. ... It specifies the maximum size for a packet before data is fragmented into multiple packets. 
 //Same as how RTS threshold works, if you notice frequent collisions on wireless network, then can consider to lower the threshold value.
  Config::SetDefault (
    "ns3::WifiRemoteStationManager::FragmentationThreshold",
    StringValue ("999999"));
  

////RTS/CTS (Request to Send / Clear to Send) is the optional mechanism used by the 802.11 (Standard WiFi) wireless networking protocol to reduce frame collisions 
  //introduced by the problem known as hidden node problem.

 //The WiFi Advanced configuration parameter RTS Threshold specifies how big packets must be for the SM to utilize the RTS (Request to Send) packet.  
  //When Uplink RTS/CTS Enabled the SM/Client must send an RTS frame and, only upon receiving a CTS frame from the AP can it transmit uplink data over the wireless
  //is allows the Access Point to schedule uplink transmission for each client helping avoid packet collisions (and resulting retransmissions / low throughput).
  Config::SetDefault (
    "ns3::WifiRemoteStationManager::RtsCtsThreshold",
    StringValue ("999999"));

//setting the drop policy such thar newest packet is dropped if the queue size becomes full
 Config::SetDefault ("ns3::WifiMacQueue::DropPolicy", EnumValue(WifiMacQueue::DROP_NEWEST));
 // Throughput
  string tplt=tcpmode+"_wifi_throughput.plt";
  string tpng=tcpmode+"_wifi_throughput.png";
 Gnuplot tplot(tpng);
 tplot.SetTerminal ("png");
 tplot.SetLegend ("PacketSize", "WifiThroughput");
 tplot.AppendExtra ("set xrange [30:1600]");
 tplot.SetTitle("WifiThroughput vs PacketSize in "+tcpmode);
 Gnuplot2dDataset tdata;
 tdata.SetTitle ("WifiThroughput");
 tdata.SetStyle (Gnuplot2dDataset::LINES_POINTS);
 
  string fplt=tcpmode+"_wifi_fairness.plt";
  string fpng=tcpmode+"_wifi_fairness.png";
 Gnuplot fplot(fpng);
 fplot.SetTerminal ("png");
 fplot.SetLegend ("PacketSize", "WifiFairness");
 fplot.AppendExtra ("set xrange [30:1600]");
 fplot.SetTitle("WifiFairness vs PacketSize in "+tcpmode);
 Gnuplot2dDataset fdata;
 fdata.SetTitle ("WifiFairness");
 fdata.SetStyle (Gnuplot2dDataset::LINES_POINTS);
 
   
         
    uint16_t port = 150;
 
    //uint32_t payloadSize[10] = {1,4,8,12,20,512,536,588,1380,1460};
    uint32_t payloadSize[10] = {40,44,48,52,60,552,576,628,1420,1500};//various payload sizes of the packets are mentioned and run in a loop
    uint32_t i=0;

    for(i=0;i<9;i++){
        NS_LOG_INFO ("Simulation for new packet size.");
       cout<<"\nPacket Size: "<<payloadSize[i]<<endl;
       //The maximum segment size (MSS) is the largest amount of data, specified in bytes, that a computer or communications device can handle in a single, unfragmented piece.
         Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (payloadSize[i]));
    NS_LOG_INFO ("Setting Up Physical and Channel");

    //create MAC layers for a ns3::WifiNetDevice
    //This class can create MACs of type ns3::ApWifiMac, ns3::StaWifiMac and ns3::AdhocWifiMac. Its purpose is to allow a WifiHelper to
    // configure and install WifiMac objects on a collection of nodes. 
    WifiMacHelper wifiMac;


    //helps to create WifiNetDevice objects
    //This class can help to create a large set of similar WifiNetDevice objects and to configure a large set of their attributes during creation.
    // Create a WifiHelper, which will use the above helpers to create
// and install Wifi devices.  Configure a Wifi standard to use, which
// will align various parameters in the Phy and Mac to standard defaults.
    WifiHelper wifiHelper;
    

    //WifiHelper::SetStandard () is a method to set various parameters in the Mac and Phy to standard values and some reasonable defaults.
    // For example, SetStandard (WIFI_PHY_STANDARD_80211a) will set the WifiPhy to Channel 36 in the 5 GHz band, among other settings appropriate for 802.11a.
     wifiHelper.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);

  //Physical devices (base class ns3::WifiPhy) connect to ns3::YansWifiChannel models in ns-3. We need to create WifiPhy objects appropriate for the YansWifiChannel; 
  //here the YansWifiPhyHelper will do the work.
  //The YansWifiPhyHelper class configures an object factory to create instances of a YansWifiPhy and adds some other objects to it, including possibly a supplemental
  // ErrorRateModel and a pointer to a MobilityModel. The user code is typically:
    YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
    

    //he RemoteStationManager to ns3::ArfWifiManager. You can change the RemoteStationManager by calling the WifiHelper::SetRemoteStationManager method.
        wifiHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue ("HtMcs5"),"ControlMode", StringValue ("HtMcs0"));
  Config::Set ("/NodeList/0/DeviceList/*/$ns3::WifiNetDevice/Phy/$ns3::WifiPhy/GuardInterval",TimeValue (NanoSeconds(800)));
    Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/ChannelWidth",UintegerValue (40));
    
YansWifiChannelHelper wifiChannel ;
    wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency", DoubleValue (5e9));//setting propagation delay and propagation loss for wifichannel

    NS_LOG_INFO ("Node Creation");
    NodeContainer node;
    node.Create (4); 
// node0 to BSS1
    Ptr<YansWifiChannel> channel1 = wifiChannel.Create ();//creating two wifi channels using wifichannel.create
//BSS2 to node1
    Ptr<YansWifiChannel> channel2 = wifiChannel.Create (); 

      


NS_LOG_INFO ("Stationary Nodes.");
  Ssid ssid = Ssid ("ns-3-ssid");


//There are presently three MAC high models that provide for the three (non-mesh; the mesh equivalent, which is a sibling of these with common parent 
  //ns3::RegularWifiMac, is not discussed here) Wi-Fi topological elements - Access Point (AP) (implemented in class ns3::ApWifiMac, non-AP Station 
  //(STA) (ns3::StaWifiMac), and STA in an Independent Basic Service Set (IBSS - also commonly referred to as an ad hoc network (ns3::AdhocWifiMac).

//The simplest of these is ns3::AdhocWifiMac`, which implements a Wi-Fi MAC that does not perform any kind of beacon generation, probing, 
  //or association. The ``ns3::StaWifiMac class implements an active probing and association state machine that handles automatic re-association whenever 
  //too many beacons are missed. Finally, ns3::ApWifiMac implements an AP that generates periodic beacons, and that accepts every attempt to associate.
wifiMac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));
wifiPhy.SetChannel (channel1);
NetDeviceContainer n0= wifiHelper.Install (wifiPhy, wifiMac, node.Get(0));
wifiPhy.SetChannel (channel2);
NetDeviceContainer n1=wifiHelper.Install (wifiPhy, wifiMac, node.Get(3));
NS_LOG_INFO ("Access points");
// Access points

    wifiMac.SetType ("ns3::ApWifiMac","Ssid", SsidValue (ssid));
    wifiPhy.SetChannel (channel1);
    NetDeviceContainer BSS1= wifiHelper.Install (wifiPhy, wifiMac, node.Get(1));
    wifiPhy.SetChannel (channel2);
    NetDeviceContainer BSS2 = wifiHelper.Install (wifiPhy, wifiMac, node.Get(2));
 
    /* Mobility model */
    NS_LOG_INFO ("Setting Mobility values");
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> location = CreateObject<ListPositionAllocator> ();

    //positions of nodes in the 3d plane
    location->Add (Vector (0.0, 0.0, 0.0));
    location->Add (Vector (100.0, 0.0, 0.0));
    location->Add (Vector (200.0, 0.0, 0.0));
    location->Add (Vector (300.0, 0.0, 0.0));  
    mobility.SetPositionAllocator (location);
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (node.Get(0));
    mobility.Install (node.Get(1));
    mobility.Install (node.Get(2));
    mobility.Install (node.Get(3));
  
    NS_LOG_INFO ("Setting Point to Point");
    PointToPointHelper p2prr;
    p2prr.SetDeviceAttribute  ("DataRate", StringValue ("10Mbps"));
    p2prr.SetChannelAttribute ("Delay", StringValue ("100ms"));
    p2prr.SetQueue ("ns3::DropTailQueue","Mode",EnumValue (DropTailQueue::QUEUE_MODE_BYTES),"MaxBytes",UintegerValue (125000));
     /* Internet stack */
    NetDeviceContainer r = p2prr.Install(node.Get(1),node.Get(2));
    InternetStackHelper stack;
    stack.Install (node);
  

    NS_LOG_INFO("IPV4 address Assigning");
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.3.1.0","255.255.255.0");
    Ipv4InterfaceContainer nr1 = ipv4.Assign(n0);
    Ipv4InterfaceContainer nr2 = ipv4.Assign(BSS1);
 
    ipv4.SetBase("10.3.2.0","255.255.255.0");
    Ipv4InterfaceContainer nr3 = ipv4.Assign(BSS2);
    Ipv4InterfaceContainer nr4 = ipv4.Assign(n1);
 
    ipv4.SetBase("10.3.3.0","255.255.255.0");
    Ipv4InterfaceContainer rr = ipv4.Assign(r);
 
    NS_LOG_INFO ("Enable static global routing.");
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  
    NS_LOG_INFO("Creating Sink");
    Address localAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
    PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory",localAddress);
   // GetAny() returns 0.0.0.0 i.e. the listen address for the sink application.
    ApplicationContainer sinkApp = packetSinkHelper.Install (node.Get (3));
 
    NS_LOG_INFO("Creating Client");
 
    OnOffHelper onoff ("ns3::TcpSocketFactory",Ipv4Address::GetAny ());
    onoff.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    onoff.SetAttribute ("PacketSize", UintegerValue (payloadSize[i]));
    onoff.SetAttribute ("DataRate", StringValue ("100Mbps")); //bit/s
  
    ApplicationContainer apps;
    AddressValue remoteAddress (InetSocketAddress (nr4.GetAddress(0), port));
    onoff.SetAttribute ("Remote", remoteAddress);
    apps.Add (onoff.Install (node.Get (0)));
 
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


