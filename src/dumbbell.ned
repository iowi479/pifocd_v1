//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package pifocd;

import inet.common.Module;
import inet.networklayer.configurator.contract.IL3NetworkConfigurator;
import inet.networklayer.configurator.contract.INetworkConfigurator;
import inet.node.ethernet.EthernetLink;
import inet.node.tsn.TsnDevice;
import inet.node.tsn.TsnSwitch;
import inet.visualizer.contract.IIntegratedVisualizer;

network DumbbellNetwork extends Module
{
    parameters:
        bool hasGlobalArp = default(true);
        *.ipv4.arp.typename = default(hasGlobalArp ? "GlobalArp" : "Arp");

    submodules:
        // ---- Sources ----
        deviceSource1: TsnDevice {
            @display("p=450,100");
        }
        deviceSource2: TsnDevice {
            @display("p=450,200");
        }
        deviceSource3: TsnDevice {
            @display("p=450,300");
        }
        deviceSource4: TsnDevice {
            @display("p=450,400");
        }

        // ---- Sinks ----
        deviceSink1: TsnDevice {
            @display("p=900,250");
        }

		// ---- Switches ----
        switch1: TsnSwitch {
            @display("p=600,150");
        }
        switch2: TsnSwitch {
            @display("p=600,350");
        }
        switch3: TsnSwitch {
            @display("p=700,250");
        }
        switch4: TsnSwitch {
            @display("p=800,250");
        }

		// ---- misc ----
        visualizer: <default(firstAvailableOrEmpty("IntegratedCanvasVisualizer"))> like IIntegratedVisualizer if typename != "" {
            @display("p=200,100;is=s");
        }
        configurator: <default("Ipv4NetworkConfigurator")> like IL3NetworkConfigurator if typename != "" {
            @display("p=200,200;is=s");
        }
        macForwardingTableConfigurator: <default(hasGlobalArp ? "MacForwardingTableConfigurator" : "")> like INetworkConfigurator if typename != "" {
            @display("p=200,300;is=s");
        }

	
    connections:
        deviceSource1.ethg++ <--> EthernetLink <--> switch1.ethg++;
        deviceSource2.ethg++ <--> EthernetLink <--> switch1.ethg++;

        deviceSource3.ethg++ <--> EthernetLink <--> switch2.ethg++;
        deviceSource4.ethg++ <--> EthernetLink <--> switch2.ethg++;

        switch1.ethg++ <--> EthernetLink <--> switch3.ethg++;
        switch2.ethg++ <--> EthernetLink <--> switch3.ethg++;

		switch3.ethg++ <--> EthernetLink <--> switch4.ethg++;

        deviceSink1.ethg++ <--> EthernetLink <--> switch4.ethg++;
}
