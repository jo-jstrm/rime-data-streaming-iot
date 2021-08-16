#!/usr/bin/python3
"""
This is the most simple example to showcase Containernet.
"""
import configparser
import os
import sys
import time

from random import shuffle, randrange
from typing import List

from mininet.net import Containernet
from mininet.node import Controller, Node
from mininet.link import TCLink
from mininet.log import setLogLevel

import experiment_args_parser


def build_topo(net: Containernet, partial_conf_dict: dict, depth, fanout, dimage='74797469/rime:latest '):
    """
    :param partial_conf_dict: current partial configuration for a {'nodename': {config}}
    If all our functions were methods (in a class), this would be accessed from `self`.
    If the PR on Containernet passed, this can be refactored. For now, pass it down the hatch.
    """
    net.hostNum = 0
    net.switchNum = 0
    net.image = dimage
    net.base_port = 3000
    net.port_idx = net.base_port + 1
    net.total_depth = depth
    net.parent_and_children = {}
    net.last_depth_nodes = []
    net.last_depth_parents = []
    _create_tree(net, partial_conf_dict, depth, fanout)


def _create_tree(net: Containernet, partial_conf_dict: dict, depth, fanout, parent_id=None):
    """Add a subtree starting with node n.
               returns: last node added"""
    isSwitch = depth > 0
    temp_parent_id = parent_id
    if isSwitch:
        node = net.addSwitch('s%s' % net.switchNum)
        net.switchNum += 1

        if depth > 0:  # add 1 node to the switch
            gateway_node = _create_new_rime_node(net, partial_conf_dict, depth, is_parent=True, parent_id=temp_parent_id)  # assume parents can have parents too
            temp_parent_id = gateway_node.name
            net.addLink(node, gateway_node)
            if depth == 1:
                net.last_depth_parents.append(temp_parent_id)

        for _ in range(fanout):  # add `fanout` nodes to switch
            child = _create_tree(net, partial_conf_dict, depth - 1, fanout, temp_parent_id)
            net.addLink(node, child, cls=TCLink, delay=_get_delay_on_depth(depth))
    else:
        node = _create_new_rime_node(net, partial_conf_dict, depth, parent_id=parent_id)
        net.last_depth_nodes.append(node.name)
    return node


def _get_delay_on_depth(depth: int) -> int:
    """
    TODO: test this in containernet
    :param depth:
    :return:
    """
    intermediate_depth_delay_min = 10
    intermediate_depth_delay_max = 30
    last_depth_delay_min = 60
    last_depth_delay_max = 150
    if depth <= 1:
        delay_min = last_depth_delay_min
        delay_max = last_depth_delay_max
    else:
        delay_min = intermediate_depth_delay_min
        delay_max = intermediate_depth_delay_max
    return randrange(delay_min, delay_max)


def _create_new_rime_node(net: Containernet, partial_conf_dict: dict, current_depth: int, is_parent=False, parent_id=None) -> Node:
    name = 'rime%s' % net.hostNum
    new_node = net.addDocker(name,
                         dimage=net.image,
                         ports=[net.base_port],
                         port_bindings={net.port_idx: net.base_port},
                         volumes=["/tmp/rime:/opt/rime/share"])  # run create-log-volume.sh
    if is_parent:
        net.parent_and_children[new_node.name] = []
    if parent_id is not None:
        net.parent_and_children[parent_id].append(new_node.name)
    partial_conf_dict[new_node.name] = _get_default_config()
    partial_conf_dict[new_node.name]['global']['tier'] = abs(net.total_depth - current_depth)  # info only available during tree build time
    partial_conf_dict[new_node.name]['global']['node_id'] = net.hostNum
    file_name = partial_conf_dict[new_node.name]['logger']['file-name']
    partial_conf_dict[new_node.name]['logger']['file-name'] = '"%s"' % (new_node.name + file_name)
    net.port_idx += 1
    net.hostNum += 1
    return new_node


def get_base_node(net: Containernet) -> Node:
    return net.getNodeByName('base')


def get_rime_nodes(net: Containernet) -> List[Node]:
    """
    Note to Johannes:
    Python3 filter returns an iterator instead of a list slicer/modifier.
    Depending on the runtime (Python2 vs Python3) this might not work.
    I'm using Python3. This is why I need list().
    """
    return list(filter(lambda host_node: 'rime' in host_node.name, net.hosts))


def get_parent_nodes(net: Containernet) -> List[Node]:
    parent_ids = net.parent_and_children.keys()
    return list(filter(lambda host_node: host_node.name in parent_ids, net.hosts))


def get_children_of_parent(net: Containernet, parent_id: str) -> List[Node]:
    children_ids = net.parent_and_children.get(parent_id)
    return list(filter(lambda host_node: host_node.name in children_ids, net.hosts))


def update_children_parent_config(net: Containernet, partial_conf_dict: dict) -> bool:
    parents = get_parent_nodes(net)
    for parent in parents:
        children = get_children_of_parent(net, parent_id=parent.name)
        for child in children:
            partial_conf_dict[child.name]['global']['relative_ip'] = '"%s"' % str(parent.IP())
    return True


def update_config_for_node(node: Node, node_current_config: dict, with_sampling_interval=None) -> bool:
    """
    Fetch and update partial config with info that is accessible from node only
    after net has initialized. These are not available during building of the topology.
    :param node: the node
    :param node_current_config: the current partial configuration dict
    :return: success/failure
    """
    am_first_root = node_current_config['global']['node_id'] == 0
    if am_first_root and node.name != "base":
        # We have to assign node0/root some IP
        node_current_config['global']['relative_ip'] = '"%s"' % str(node.IP())    
            
    node_current_config['global']['my_ip'] = '"%s"' % str(node.IP())
    node_current_config['global']['am_first_root'] = str(am_first_root).lower()
    if with_sampling_interval is not None:
        node_current_config['global']['sampling_interval'] = with_sampling_interval
    
    return node.name, node_current_config, am_first_root


def write_config_for_node(node_name: str, node_config: dict):
    with open("/tmp/rime/config" + os.sep + node_name + '.ini', 'w') as node_settings:
        config = configparser.ConfigParser(interpolation=None)
        config.read_dict(node_config)
        config.write(node_settings, space_around_delimiters=False)
        return True
    return False


def create_config_for_base(base_node: Node) -> dict:
    """
    Handle creation of partial config (before net init) for base node.
    Post-init will be handled the same way for all nodes from `update_and_write_config_for_node`.
    :param base_node: the node used as a base for the network
    :return: dict with partial information, to be appended to total partial_config
    """
    base_config = _get_default_config()
    file_name = base_config['logger']['file-name']
    base_config['logger']['file-name'] = '"%s"' % (base_node.name + file_name)
    base_config['global']['base-station-mode'] = "true"
    return base_config


def get_random_movers(movers: int) -> List[str]:
        mover_ids = []
        for _ in range(movers):
            id = randrange(len(rime_nodes))
            mover_ids.append('rime' + str(id))
        return mover_ids


def perform_random_failure(net: Containernet, failure_num=1):
    last_depth_parents = net.last_depth_parents
    shuffle(last_depth_parents)
    for idx in range(failure_num):
        node = net.getNodeByName(last_depth_parents[idx])
        net.delHost(node)


def _get_default_config() -> dict:
    return {
        'global': {
            "my_ip": "",
            "my_port": 3000,
            "node_id": -1,            
            "relative_ip": "",
            "relative_port": 3000,
            "am_first_root": "false",
            "sampling_interval": 500            
        },
        'logger': {
            "file-name": "_actor_log_[PID]_[TIMESTAMP]_[NODE].log",
            "file-format": '"%r %c %p %a %t %C %M %F:%L %m%n"',
            "file-verbosity": "'error'"
        },
        'middleman': {
            "enable-automatic-connections": "true"
        }
    }


if __name__ == '__main__':
    parser = experiment_args_parser.create_parser()
    args = parser.parse_args()

    depth = args.depth  # depth of tree
    fanout = args.fanout  # num of nodes at gateway switches (+1 for gateway nodes)
    actual_sampling_interval = args.sampling_interval  # the sampling interval at the node level (ms)
    sleep_time = args.sleep_time  # the amount of sleep time in seconds
    send_query = args.send_query
    query_x = args.query_x
    query_y = args.query_y
    failure = args.failure
    movers = args.movers
    docker_img = args.baseline

    if not os.path.exists("/tmp/rime"):
        os.makedirs("/tmp/rime/logs")
        os.makedirs("/tmp/rime/config")

    base_ip = '10.0.0.1'
 
    setLogLevel('error')

    # info('*** Adding network\n')
    net = Containernet(controller=Controller)
    net.addController('c0')
    partial_config = {}

    # info('*** Adding docker containers\n')
    base = net.addDocker('base', ip=base_ip, dimage=docker_img, volumes=["/tmp/rime:/opt/rime/share"])
    build_topo(net, partial_conf_dict=partial_config, depth=depth, fanout=fanout, dimage=docker_img)
    net.addLink(base, net.switches[0])
    partial_config[base.name] = create_config_for_base(base)
    if send_query:
        partial_config[base.name]['global']['send_query'] = "true"
        partial_config[base.name]['global']['query_x'] = query_x
        partial_config[base.name]['global']['query_y'] = query_y

    # info('*** Starting net\n')
    net.start()

    # info('*** Updating child-and-parent config\n')
    update_children_parent_config(net, partial_config)
    hosts = net.hosts

    # info('*** Starting rime processes\n')
    binary_suffix = "" if docker_img == "74797469/rime:latest" else "-baseline"
    cmd_prefix = f'cd /opt/rime/share/logs && /opt/rime/build/rime{binary_suffix} --config-file=/opt/rime/share/config/'
    cmd_suffix = '.ini &'

    rime_nodes = get_rime_nodes(net)
    rime_nodes.append(base)

    if movers > 0:
        mover_ids = get_random_movers(movers)

    for node in rime_nodes:
        node_name, node_config, am_first_root = update_config_for_node(node, partial_config[node.name],
                                                                       with_sampling_interval=actual_sampling_interval)

        if am_first_root:
            partial_config[base.name]['global']['relative_ip'] = node_config['global']['my_ip']
                
        if movers > 0 and node_name in mover_ids:
            partial_config[node_name]['global']['start_moving'] = "true"

        write_config_for_node(node_name, node_config)
        node.cmd(cmd_prefix + node.name + cmd_suffix)

    if failure:  # ¯\_(ツ)_/¯
        time.sleep(sleep_time / 2)
        perform_random_failure(net)
        time.sleep(sleep_time / 2)
    else:
        time.sleep(sleep_time)

    # info('*** Stopping network')
    net.stop()

    sys.exit(0)
