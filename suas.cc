
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/energy-module.h"
#include "ns3/internet-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
//#include "ns3/config-store-module.h"
#include "ns3/netanim-module.h"
#include "ns3/animation-interface.h"
#include "ns3/applications-module.h"

#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/animation-interface.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/uinteger.h"
#include "ns3/mobility-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/wifi-module.h"
#include "ns3/gnuplot.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/aodv-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-mac.h"
#include "ns3/pointer.h"
#include "ns3/boolean.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/animation-interface.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/uinteger.h"
#include "ns3/mobility-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/wifi-module.h"
#include "ns3/gnuplot.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/aodv-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-mac.h"
#include "ns3/pointer.h"
#include "ns3/boolean.h"



//
//#include "crypto++/osrng.h"
//
//#include "crypto++/filters.h"
//using CryptoPP::StringSink;
//using CryptoPP::StringSource;
//using CryptoPP::StreamTransformationFilter;

//#include "crypto++/hex.h"
//
//#include "crypto++/rc5.h"
//
//#include "crypto++/aes.h"
//using CryptoPP::AES;

//#include "crypto++/cryptlib.h"

//#include "crypto++/ccm.h"
//using CryptoPP::CBC_Mode;

#include "assert.h"

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <fstream>
#include <vector>
#include <string>
using std::string;



NS_LOG_COMPONENT_DEFINE ("Suastika");

using namespace ns3;
//using namespace CryptoPP;

static inline std::string
PrintReceivedPacket (Address& from)
{
  InetSocketAddress iaddr = InetSocketAddress::ConvertFrom (from);

  std::ostringstream oss;
  oss << "--\nReceived one packet! Socket: " << iaddr.GetIpv4 ()
      << " port: " << iaddr.GetPort ()
      << " at time = " << Simulator::Now ().GetSeconds ()
      << "\n--";

  return oss.str ();
}

double energyConstant = 0;
/**
 * \param socket Pointer to socket.
 *
 * Packet receiving sink.
 */
void
ReceivePacket (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {
      if (packet->GetSize () > 0)
        {
          NS_LOG_UNCOND (PrintReceivedPacket (from));
        }
    }
}

/**
  
 * Traffic generator.
 */
static void
GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize, Ptr<Node> n,
                 uint32_t pktCount, Time pktInterval)
{
  if (pktCount > 0)
    {
      socket->Send (Create<Packet> (pktSize));
      Simulator::Schedule (pktInterval, &GenerateTraffic, socket, pktSize, n,
                           pktCount - 1, pktInterval);
    }
  else
    {
      socket->Close ();
    }
}

/// Trace function for remaining energy at node.
void
RemainingEnergy (double oldValue, double remainingEnergy)
{
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds ()
                 << "s Current remaining energy = " << remainingEnergy-(energyConstant*Simulator::Now ().GetSeconds ()) << "J");
}

/// Trace function for total energy consumption at node.
void
TotalEnergy (double oldValue, double totalEnergy)
{
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds ()
                 << "s Total energy consumed by radio = " << totalEnergy+(energyConstant*Simulator::Now ().GetSeconds ()) << "J");
}

int
main (int argc, char *argv[])
{
 
  std::string phyMode ("DsssRate2Mbps");
  double Prss = -80;            // dBm
  uint32_t PpacketSize = 200;   // bytes
  bool verbose = false;

double distance = 100;
  uint32_t fakeNode = 1;

//  bool tracing = false;

  // simulation parameters
  uint32_t numPackets = 100;  // number of packets to send
  double interval = 1;          // seconds
  double startTime = 0.0;       // seconds
  double distanceToRx = 100.0;  // meters
  /*
   * This is a magic number used to set the transmit power, based on other
   * configuration.
   */
  double offset = 81;

  CommandLine cmd;
  cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
  cmd.AddValue ("Prss", "Intended primary RSS (dBm)", Prss);
  cmd.AddValue ("PpacketSize", "size of application packet sent", PpacketSize);
  cmd.AddValue ("numPackets", "Total number of packets to send", numPackets);
  cmd.AddValue ("startTime", "Simulation start time", startTime);
  cmd.AddValue ("distanceToRx", "X-Axis distance between nodes", distanceToRx);
  cmd.AddValue ("verbose", "Turn on all device log components", verbose);
  cmd.Parse (argc, argv);

  // Convert to time object
  Time interPacketInterval = Seconds (interval);

  // disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold",
                      StringValue ("2200"));
  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold",
                      StringValue ("2200"));
  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
                      StringValue (phyMode));

 

  string plain = "Test Pesan";
  cout << "pesan: " << plain << endl;
 
  NodeContainer c;
  c.Create (10);     // create 2 nodes
  NodeContainer networkNodes;
  networkNodes.Add (c.Get (0));
  networkNodes.Add (c.Get (1));
  networkNodes.Add (c.Get (2));
  networkNodes.Add (c.Get (3));
  networkNodes.Add (c.Get (4));
  networkNodes.Add (c.Get (5));
  networkNodes.Add (c.Get (6));
  networkNodes.Add (c.Get (7));
  networkNodes.Add (c.Get (8));
  networkNodes.Add (c.Get (9));

  NodeContainer f;
  f.Create(fakeNode);


  // The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi;
  if (verbose)
    {
      wifi.EnableLogComponents ();
    }
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

  /** Wifi PHY **/
  /***************************************************************************/
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  wifiPhy.Set ("RxGain", DoubleValue (-10));
//  wifiPhy.Set ("TxGain", DoubleValue (offset + Prss));
  wifiPhy.Set ("TxGain", DoubleValue (offset + Prss-15.0));
  wifiPhy.Set ("CcaMode1Threshold", DoubleValue (0.0));
  /***************************************************************************/

  /** wifi channel **/
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  // create wifi channel
  Ptr<YansWifiChannel> wifiChannelPtr = wifiChannel.Create ();
  wifiPhy.SetChannel (wifiChannelPtr);

  /** MAC layer **/
  // Add a non-QoS upper MAC, and disable rate control
  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
                                StringValue (phyMode), "ControlMode",
                                StringValue (phyMode));
  // Set it to ad-hoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac");

  /** install PHY + MAC **/
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, networkNodes);
NetDeviceContainer fakeDevices = wifi.Install(wifiPhy, wifiMac, f);


   
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (distance),
                                 "DeltaY", DoubleValue (distance),
                                 "GridWidth", UintegerValue (5),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (c);
  mobility.Install(f);

  // Enable OLSR
  AodvHelper aodv;
Ipv4StaticRoutingHelper staticRouting;

  Ipv4ListRoutingHelper list;
    list.Add (staticRouting, 0);
    list.Add (aodv, 10);
InternetStackHelper internet;
    internet.SetRoutingHelper (list);
    internet.Install (networkNodes);

  InternetStackHelper aodvInternet;
  aodvInternet.SetRoutingHelper(aodv);
  aodvInternet.Install(f);
  
  /** Energy Model **/
  /***************************************************************************/
  /* energy source */
  BasicEnergySourceHelper basicSourceHelper;
  // configure energy source
  basicSourceHelper.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (1500.0));
  // install source
  EnergySourceContainer sources = basicSourceHelper.Install (c);
  /* device energy model */
  WifiRadioEnergyModelHelper radioEnergyHelper;
  // configure radio energy model
  radioEnergyHelper.Set ("TxCurrentA", DoubleValue (0.00000));
  // install device model
  DeviceEnergyModelContainer deviceModels = radioEnergyHelper.Install (devices, sources);
  /***************************************************************************/

  /** Internet stack **/
//  InternetStackHelper internet;
//  internet.Install (networkNodes);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devices);
Ipv4InterfaceContainer j = ipv4.Assign(fakeDevices);
  UdpEchoServerHelper echoServer(91);

  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink = Socket::CreateSocket (networkNodes.Get (9), tid);  // node 14, receiver
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
  recvSink->Bind (local);
  recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

  Ptr<Socket> source = Socket::CreateSocket (networkNodes.Get (0), tid);    // node 0, sender
  InetSocketAddress remote = InetSocketAddress (i.GetAddress(9,0), 80);
  source->SetAllowBroadcast (true);
  source->Connect (remote);

  //-------------------------------------------------------------------------
  ApplicationContainer serverApps =  echoServer.Install(c.Get(9));
  serverApps.Start(Seconds(1.0));
  serverApps.Stop(Seconds(1500.0));

  UdpEchoClientHelper echoClient(i.GetAddress(9), 91);
  echoClient.SetAttribute("MaxPackets", UintegerValue(1000));
  echoClient.SetAttribute("Interval", TimeValue(Seconds(2.0)));

  ApplicationContainer client_app_0 = echoClient.Install(c.Get(0));
  client_app_0.Start(Seconds(2.0));
  client_app_0.Stop(Seconds(1500.0));

  ApplicationContainer client_app_1 = echoClient.Install(c.Get(1));
  client_app_1.Start(Seconds(2.0));
  client_app_1.Stop(Seconds(1500.0));

  ApplicationContainer client_app_2 = echoClient.Install(c.Get(2));
  client_app_2.Start(Seconds(2.0));
  client_app_2.Stop(Seconds(1500.0));

  ApplicationContainer client_app_3 = echoClient.Install(c.Get(3));
  client_app_3.Start(Seconds(2.0));
  client_app_3.Stop(Seconds(1500.0));

  ApplicationContainer client_app_4 = echoClient.Install(c.Get(4));
  client_app_4.Start(Seconds(2.0));
  client_app_4.Stop(Seconds(1500.0));

  ApplicationContainer client_app_5 = echoClient.Install(c.Get(5));
  client_app_5.Start(Seconds(2.0));
  client_app_5.Stop(Seconds(1500.0));

  ApplicationContainer client_app_6 = echoClient.Install(c.Get(6));
  client_app_6.Start(Seconds(2.0));
  client_app_6.Stop(Seconds(1500.0));

  ApplicationContainer client_app_7 = echoClient.Install(c.Get(7));
  client_app_7.Start(Seconds(2.0));
  client_app_7.Stop(Seconds(1500.0));

  ApplicationContainer client_app_8 = echoClient.Install(c.Get(8));
  client_app_8.Start(Seconds(2.0));
  client_app_8.Stop(Seconds(1500.0));

  ApplicationContainer client_app_9 = echoClient.Install(c.Get(9));
  client_app_9.Start(Seconds(2.0));
  client_app_9.Stop(Seconds(1500.0));

  //----------------connecting fakeNodes to network-------------------
  UdpEchoClientHelper fakeClient(i.GetAddress(4), 91);
  fakeClient.SetAttribute("MaxPackets", UintegerValue(1000));
  fakeClient.SetAttribute("Interval", TimeValue(Seconds(2.0)));

  ApplicationContainer fake_app_0 = fakeClient.Install(f.Get(0));
  fake_app_0.Start(Seconds(2.0));
  fake_app_0.Stop(Seconds(200.0));


  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  //-implement encryption-
  echoClient.SetFill(client_app_0.Get(0), plain);
  echoClient.SetFill(client_app_1.Get(0), plain);
  echoClient.SetFill(client_app_2.Get(0), plain);
  echoClient.SetFill(client_app_3.Get(0), plain);
  echoClient.SetFill(client_app_4.Get(0), plain);
  echoClient.SetFill(client_app_5.Get(0), plain);
  echoClient.SetFill(client_app_6.Get(0), plain);
  echoClient.SetFill(client_app_7.Get(0), plain);
  echoClient.SetFill(client_app_8.Get(0), plain);
  echoClient.SetFill(client_app_9.Get(0), plain);

  /*-----------------------------------------------*/
  fakeClient.SetFill(fake_app_0.Get(0), "Pesan kebakaran");



  /** connect trace sources **/
  /***************************************************************************/
  // all sources are connected to node (1)
  // energy source
  Ptr<BasicEnergySource> basicSourcePtr = DynamicCast<BasicEnergySource> (sources.Get (1));
  basicSourcePtr->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback (&RemainingEnergy));

  // device energy model
  Ptr<DeviceEnergyModel> basicRadioModelPtr =
  basicSourcePtr->FindDeviceEnergyModels ("ns3::WifiRadioEnergyModel").Get (0);
  NS_ASSERT (basicRadioModelPtr != NULL);
  basicRadioModelPtr->TraceConnectWithoutContext ("TotalEnergyConsumption", MakeCallback (&TotalEnergy));

  /***************************************************************************/

  //Output
  AsciiTraceHelper ascii;
  wifiPhy.EnableAsciiAll(ascii.CreateFileStream("wireless_no_encrypt10.tr"));
  wifiPhy.EnablePcap("Pcap_Suastika222", devices);

  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>("rute", std::ios::out);
  //olsr.PrintRoutingTableAllEvery(Seconds(2.0),routingStream);

  /*Animation setup*/
  AnimationInterface anm("Suastika.xml");


  /*Setting titik di NetAnim*/
  anm.SetConstantPosition(c.Get(0),100.0, 100.0);
  anm.SetConstantPosition(c.Get(1),200.0, 50.0);
  anm.SetConstantPosition(c.Get(2),250.0, 85.0);
  anm.SetConstantPosition(c.Get(3),200.0, 150.0);
  anm.SetConstantPosition(c.Get(4),275.0, 25.0);
  anm.SetConstantPosition(c.Get(5),135.0, 315.0);
  anm.SetConstantPosition(c.Get(6),175.0, 200.0);
  anm.SetConstantPosition(c.Get(7),100.0, 325.0);
  anm.SetConstantPosition(c.Get(8),375.0, 115.0);
  anm.SetConstantPosition(c.Get(9),265.0, 175.0);


  //-------------------------------------------------

  anm.SetConstantPosition(f.Get(0), 200.0, 177.0);
anm.SetConstantPosition(f.Get(0), 200.0, 177.0);


  /** simulation setup **/
  // start traffic
  Simulator::Schedule (Seconds (startTime), &GenerateTraffic, source, PpacketSize,
                       networkNodes.Get (0), numPackets, interPacketInterval);

  Simulator::Stop (Seconds (1500.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
