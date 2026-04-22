from pox.core import core
import pox.openflow.libopenflow_01 as of

log = core.getLogger()

def _handle_PacketIn(event):
    packet = event.parsed

    msg = of.ofp_packet_out()
    msg.data = event.ofp

    if packet.dst.is_broadcast:
        log.info("Broadcast detected")

        # Allow broadcast to all ports (safe for ARP)
        msg.actions.append(of.ofp_action_output(port=of.OFPP_FLOOD))

    else:
        # Normal forwarding
        msg.actions.append(of.ofp_action_output(port=of.OFPP_FLOOD))

    event.connection.send(msg)

def launch():
    core.openflow.addListenerByName("PacketIn", _handle_PacketIn)
