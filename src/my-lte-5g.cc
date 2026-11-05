/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
// 3 eNB (сектора), 100 UE, три профиля нагрузки.
// enbConfig=foreign|domestic задаёт конфигурацию eNB до старта.
// Миграция моделируется двумя последовательными прогонами (план B),
// так как LteEnbPhy в ns-3 LTE не пересчитывает TxPower/NF в runtime.

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("MyLte5g");

int main(int argc, char *argv[])
{
    uint32_t seed      = 1000;
    std::string load   = "low";
    std::string enbCfg = "foreign";
    double   txPowerOverride = -1.0;
    double   noiseOverride   = -1.0;
    uint32_t nUe       = 100;
    double   simTime   = 6 * 3600.0;

    CommandLine cmd;
    cmd.AddValue("seed",      "RNG seed",                      seed);
    cmd.AddValue("load",      "Load level (low/med/high)",     load);
    cmd.AddValue("enbConfig", "eNB config (foreign/domestic)", enbCfg);
    cmd.AddValue("txPower", "Override domestic TxPower (dBm)", txPowerOverride);
    cmd.AddValue("noise",   "Override domestic NF (dB)",      noiseOverride);
    cmd.AddValue("nUe",       "Number of UEs",                 nUe);
    cmd.AddValue("simTime",   "Simulation time (s)",           simTime);
    cmd.Parse(argc, argv);

    for (double t = 60.0; t <= simTime; t += 60.0) {
        Simulator::Schedule(Seconds(t), []() {
            std::cout << "t=" << Simulator::Now().GetSeconds() << "s" << std::endl;
        });
    }

    RngSeedManager::SetSeed(seed);
    RngSeedManager::SetRun(seed);

        double enbTxPower = 43.0, enbNoise = 5.0;
    if (enbCfg == "domestic") {
        enbTxPower = (txPowerOverride > 0) ? txPowerOverride : 35.0;
        enbNoise   = (noiseOverride   > 0) ? noiseOverride   : 7.0;
    } else if (enbCfg != "foreign") {
        NS_FATAL_ERROR("Unknown enbConfig: " << enbCfg);
    }

    Config::SetDefault("ns3::LteEnbPhy::TxPower",     DoubleValue(enbTxPower));
    Config::SetDefault("ns3::LteEnbPhy::NoiseFigure", DoubleValue(enbNoise));
    Config::SetDefault("ns3::LteUePhy::TxPower",      DoubleValue(23.0));
    Config::SetDefault("ns3::LteUePhy::NoiseFigure",  DoubleValue(9.0));

    Ptr<LteHelper> lte = CreateObject<LteHelper>();
    lte->SetAttribute("PathlossModel",
                      StringValue("ns3::LogDistancePropagationLossModel"));
    Ptr<PointToPointEpcHelper> epc = CreateObject<PointToPointEpcHelper>();
    lte->SetEpcHelper(epc);
    Ptr<Node> pgw = epc->GetPgwNode();

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("10Gbps"));
    p2p.SetChannelAttribute("Delay",    StringValue("1ms"));

    NodeContainer remoteHostContainer; remoteHostContainer.Create(1);
    Ptr<Node> remoteHost = remoteHostContainer.Get(0);
    InternetStackHelper internet; internet.Install(remoteHostContainer);
    NetDeviceContainer internetDevices = p2p.Install(pgw, remoteHost);

    Ipv4AddressHelper ipv4h; ipv4h.SetBase("1.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign(internetDevices);
    Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress(1);
    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> rh = ipv4RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv4>());
    rh->AddNetworkRouteTo(Ipv4Address("7.0.0.0"), Ipv4Mask("255.0.0.0"), 1);

    NodeContainer enbNodes; enbNodes.Create(3);
    NodeContainer ueNodes;  ueNodes.Create(nUe);

        Ptr<ListPositionAllocator> enbPos = CreateObject<ListPositionAllocator>();
    enbPos->Add(Vector( 250.0,   0.0, 30.0));
    enbPos->Add(Vector(-250.0,   0.0, 30.0));
    enbPos->Add(Vector(   0.0, 433.0, 30.0));
    MobilityHelper enbMob;
    enbMob.SetPositionAllocator(enbPos);
    enbMob.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    enbMob.Install(enbNodes);

        Ptr<UniformDiscPositionAllocator> uePos = CreateObject<UniformDiscPositionAllocator>();
    uePos->SetX(0.0);
    uePos->SetY(150.0);
    uePos->SetRho(700.0);
    MobilityHelper ueMob;
    ueMob.SetPositionAllocator(uePos);
    ueMob.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                "Bounds", RectangleValue(Rectangle(-800, 800, -600, 1000)),
        "Speed",  StringValue("ns3::UniformRandomVariable[Min=0.0|Max=0.83]"));
    ueMob.Install(ueNodes);

     NetDeviceContainer enbDevs = lte->InstallEnbDevice(enbNodes);
    for (uint32_t i = 0; i < enbDevs.GetN(); ++i) {
        Ptr<LteEnbNetDevice> dev = enbDevs.Get(i)->GetObject<LteEnbNetDevice>();
        Ptr<LteEnbPhy> phy = dev->GetPhy();
        phy->SetAttribute("TxPower",     DoubleValue(enbTxPower));
        phy->SetAttribute("NoiseFigure", DoubleValue(enbNoise));
        std::cout << "eNB " << (i + 1)
                  << " TxPower=" << phy->GetTxPower()
                  << " dBm, NF="   << phy->GetNoiseFigure()
                  << " dB" << std::endl;
    }
    NetDeviceContainer ueDevs  = lte->InstallUeDevice(ueNodes);

    internet.Install(ueNodes);
    Ipv4InterfaceContainer ueIpIfaces = epc->AssignUeIpv4Address(ueDevs);
    for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {
        Ptr<Ipv4StaticRouting> uer = ipv4RoutingHelper.GetStaticRouting(
            ueNodes.Get(u)->GetObject<Ipv4>());
        uer->SetDefaultRoute(epc->GetUeDefaultGatewayAddress(), 1);
    }
    lte->Attach(ueDevs);

    uint32_t nActive = 20; std::string rate = "200kbps";
    if (load == "low")       { nActive = 20;  rate = "200kbps"; }
    else if (load == "med")  { nActive = 50;  rate = "500kbps"; }
    else if (load == "high") { nActive = 100; rate = "1Mbps";   }
    else NS_FATAL_ERROR("Unknown load: " << load);

        uint16_t port = 1234;

    // --- Sinks на UE: принимают DL-трафик ---
    ApplicationContainer ueSinkApps;
    for (uint32_t u = 0; u < nActive; ++u) {
        PacketSinkHelper ueSink("ns3::UdpSocketFactory",
                                InetSocketAddress(Ipv4Address::GetAny(), port));
        ApplicationContainer sa = ueSink.Install(ueNodes.Get(u));
        sa.Start(Seconds(0.0));
        sa.Stop(Seconds(simTime));
        ueSinkApps.Add(sa);
    }

    // --- OnOff на remoteHost: шлёт DL-трафик на UE ---
    for (uint32_t u = 0; u < nActive; ++u) {
        OnOffHelper onoff("ns3::UdpSocketFactory",
                          InetSocketAddress(ueIpIfaces.GetAddress(u), port));
        onoff.SetAttribute("OnTime",  StringValue("ns3::ConstantRandomVariable[Constant=3600]"));
        onoff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.001]"));
        onoff.SetAttribute("DataRate", StringValue(rate));
        onoff.SetAttribute("PacketSize", UintegerValue(1024));
        ApplicationContainer app = onoff.Install(remoteHost);
        app.Start(Seconds(1.0));
        app.Stop(Seconds(simTime));
    }

    Simulator::Stop(Seconds(simTime));
    Simulator::Run();

       double rxBytes = 0.0;
    uint32_t ueWithTraffic = 0;
    for (ApplicationContainer::Iterator it = ueSinkApps.Begin(); it != ueSinkApps.End(); ++it) {
        Ptr<PacketSink> s = DynamicCast<PacketSink>(*it);
        if (s && s->GetTotalRx() > 0) ueWithTraffic++;
    }
    std::cout << "UE_with_traffic=" << ueWithTraffic << "/" << nActive << std::endl;
    for (ApplicationContainer::Iterator it = ueSinkApps.Begin(); it != ueSinkApps.End(); ++it) {
        Ptr<PacketSink> s = DynamicCast<PacketSink>(*it);
        if (s) rxBytes += s->GetTotalRx();
    }
    double dur = simTime - 1.0;
    std::cout << "\n=== Summary ===" << std::endl;
    std::cout << "seed="      << seed     << std::endl;
    std::cout << "load="      << load     << std::endl;
    std::cout << "enbConfig=" << enbCfg   << std::endl;
    std::cout << "nActive="   << nActive  << std::endl;
    std::cout << "RxBytes="   << rxBytes  << std::endl;
    std::cout << "Throughput(Mbps)=" << (rxBytes * 8.0 / dur / 1e6) << std::endl;

    Simulator::Destroy();
    return 0;
}
