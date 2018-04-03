# -*- coding: utf-8 -*-

"""* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  Network Class                                                                                                      *
 *  Network Generator                                                                                                  *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 10/02/18.                                                                             *
 *  Copyright © 2018 Francisco Pozo. All rights reserved.                                                              *
 *  Class with the information of the network and algorithms to create them. It uses the package NetworkX.             *
 *  Networks are generated with different manners. An easy an fast description language for networks without cycles, a *
 *  more normal connection by connection approach for any topology, or some defined network algorithms from NetworkX   *
 *  Different number of frames and types of frames frames (single, broadcast, etc) and dependencies and attributes of  *
 *  the network are also created.                                                                                      *
 *  As the number of parameters is large, standard configuration of parameters are also available.                     *
 *  It includes input and output for the xml files.                                                                    *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * """

from enum import Enum
import xml.etree.ElementTree as Xml
from xml.dom import minidom
from Frame import Frame
from Link import Link
from Node import Node
import networkx as nx
from random import random, choice, shuffle, randint


class Network:
    """
    Network class with the information of the whole network, frames and dependencies that contains. It includes
    algorithms to construct them. Parameters can be read from a XML file and the resulting network is written in another
    XML file.
    """

    class TimeUnit(Enum):
        """
        Time Unit class to to take different units
        """
        ns = 1  # Nanoseconds
        us = 2  # Microseconds
        ms = 3  # Miliseconds
        s = 4  # Seconds

        @staticmethod
        def from_string(string):
            """
            Converts a given string to a TimeUnit
            :param string: string with the time unit
            :type string: str
            :return: time unit converted
            :rtype: Network.TimeUnit
            """
            if not isinstance(string, str):
                raise TypeError('The given string should be a string')
            if string == 'ns':
                return Network.TimeUnit.ns
            if string == 'us':
                return Network.TimeUnit.us
            if string == 'ms':
                return Network.TimeUnit.ms
            if string == 's':
                return Network.TimeUnit.s
            raise ValueError('The given string should be in the TimeUnit class')

    class SpeedUnit(Enum):
        """
        Speed Unit class to take different speeds
        """
        kbs = 1     # KB/s
        mbs = 2     # MB/s
        gbs = 3     # GB/s

        @staticmethod
        def from_string(string):
            """
            Converts a given string to a SpeedUnit
            :param string: string with the speed unit
            :type string: str
            :return: speed unit converted
            :rtype: Network.SpeedUnit
            """
            if not isinstance(string, str):
                raise TypeError('The given string should be a string')
            if string == 'KB/s':
                return Network.SpeedUnit.kbs
            if string == 'MB/s':
                return Network.SpeedUnit.mbs
            if string == 'GB/s':
                return Network.SpeedUnit.gbs
            raise ValueError('The given string should be in the SpeedUnit class')

    class SelfHealingProtocol:
        """
        Self-Healing Protocol class to save its information
        """

        def __init__(self, period, time):
            """
            Init the Self-Healing Protocol Bandwidth reservation
            :param period: period in ns
            :type period: int
            :param time: time in ns
            :type time: int
            """
            self.__set_period(period)
            self.__set_time(time)

        def __str__(self):
            """
            Get as string the information of the self-healing protocol
            :return: the self-healing protocol information
            :rtype: str
            """
            return_text = 'The self-healing protocol information =>\n'
            return_text += "    Period     : " + str(self.period) + " ns\n"
            return_text += "    Time       : " + str(self.time) + " ns"
            return return_text

        def __set_period(self, period):
            """
            Set the period of the self-healing protocol bandwidth in ns
            :param period: period of the self-healing protocol bandwidth reservation in ns
            :type period: int
            """
            if not isinstance(period, int):
                raise TypeError('The period of the self-healing protocol bandwidth reservation should be an integer')
            if period <= 0:
                raise ValueError('The period of the self-healing protocol bandwidth reservation should be natural')
            self.__period = period

        def __get_period(self):
            """
            Get the period of the self-healing protocol bandwidth in ns
            :return: the period of the self-healing bandwidth in ns
            :rtype: int
            """
            return self.__period

        def __set_time(self, time):
            """
            Set the time of the self-healing protocol bandwidth in ns
            :param time: time of the self-healing protocol bandwidth reservation in ns
            :type time: int
            """
            if not isinstance(time, int):
                raise TypeError('The time of the self-healing protocol bandwidth reservation should be an integer')
            if time <= 0:
                raise ValueError('The time of the self-healing protocol bandwidth reservation should be natural')
            if time >= self.__period:
                raise ValueError('The time of the self-healing protocol cannot be larger than its period')
            self.__time = time

        def __get_time(self):
            """
            Get the time of the self-healing protocol bandwidth in ns
            :return: the time of the self-healing bandwidth in ns
            :rtype: int
            """
            return self.__time

        period = property(__get_period, __set_period)
        time = property(__get_time, __set_time)

    class Topology:
        """
        Topology class that saves the information needed to create an specific network.
        This class is able to store information of different ways to define the network topology
        """

        class TopologyType(Enum):
            """
            Different topology types we can support
            """
            non_cyclic = 1          # Without cycles, only one possible path from one end system to another end system
            configurable = 2        # The user has to specify manually all the nodes and connections

            @staticmethod
            def from_string(string):
                """
                Converts a given string to a TopologyType
                :param string: string with the topology type
                :type string: str
                :return: topology type converted
                :rtype: TopologyType
                """
                if not isinstance(string, str):
                    raise TypeError('The given string should be a string')
                if string == 'non-cyclic':
                    return Network.Topology.TopologyType.non_cyclic
                if string == 'configurable':
                    return Network.Topology.TopologyType.configurable
                raise ValueError('The given string should be in the TopologyType class')

        def __init__(self, topology_type):
            """
            Init the topology, we only set the topology type at the start to know which values we should be adding.
            It helps us to see, after init the class, if we are getting information that does not fit our topology type
            :param topology_type: type of our topology
            :type topology_type: Network.Topology.TopologyType
            """
            self.__set_topology_type(topology_type)

        def __set_topology_type(self, topology_type):
            """
            Set the topology type of our topology, needed to know which information we need to look into
            :param topology_type: type of our topology
            :type topology_type: Network.Topology.TopologyType
            """
            if not isinstance(topology_type, Network.Topology.TopologyType):
                raise TypeError('The topology_type should be a TopologyType object')
            self.__topology_type = topology_type

        def __get_topology_type(self):
            """
            Get the topology type
            :return: type of our topology
            :rtype: Network.Topology.TopologyType
            """
            return self.__topology_type

        def __set_non_cyclic_description(self, description):
            """
            Set the network describing the non-cyclic topology.
            The topology description string with the data of the network, it follows an special description
            There are numbers divided by semicolons, every number indicates the number of children for the actual switch
            If the number is negative, it means it has x end systems: ex: -5 means 5 end systems
            If the number is 0, it means the actual switch is actually an end_system
            The order of the descriptions goes with depth. If a node has no more children you backtrack to describe the
            next node.
            :param description: string describing the topology
            :type description: str
            """
            if not isinstance(description, str):
                raise TypeError('The description should be a string')
            if self.__topology_type != Network.Topology.TopologyType.non_cyclic:
                raise TypeError('This description can only called if the topology type is non-cyclic')
            self.__non_cyclic_description = description

        def __get_non_cyclic_description(self):
            """
            Get the non-cyclic description string
            :return: non-cyclic description string
            :rtype: str
            """
            return self.__non_cyclic_description

        def __set_non_cyclic_link_description(self, description):
            """
            Set the information of the links in the non-cyclic topology.
            Every link has a description of its speed and its type (wired/wireless), first letter is the type and number
            w100 => wired with 100 MBs
            x10 => wireless with 10 MBs
            :param description: string with the information of the links
            :type description: str
            """
            if not isinstance(description, str):
                raise TypeError('The description should be a string')
            if self.__topology_type != Network.Topology.TopologyType.non_cyclic:
                raise TypeError('This description can only called if the topology type is non-cyclic')
            self.__non_cyclic_link_description = description

        def __get_non_cyclic_link_description(self):
            """
            Get the string with the information of the link description
            :return: string with the information of the link description
            :rtype: str
            """
            return self.__non_cyclic_link_description

        def __set_configurable_description(self, description):
            """
            Set the description of the configurable topology.
            The information is a list of nodes with the current information: node id, node type, list of node ids
            connections, list of links to connect to each node
            :param description: list with node id, node type, list of node ids connections, list of links to connect
            :type description: list of [int, Node.NodeType, list of int, list of Link]
            """
            if not isinstance(description, list):
                raise TypeError('The description is not a list')
            for node_description in description:
                if not isinstance(node_description[0], int):
                    raise TypeError('The node id is not an integer')
                if not isinstance(node_description[1], Node.NodeType):
                    raise TypeError('The node type of ' + str(node_description[0] + ' should be a NodeType class'))
                if not isinstance(node_description[2], list):
                    raise TypeError('The node connections of ' + str(node_description[0]) + ' should be list')
                for connection in node_description[2]:
                    if not isinstance(connection, int):
                        raise TypeError('A node connections of ' + str(node_description[0]) + ' should be an integer')
                if not isinstance(node_description[3], list):
                    raise TypeError('The link information of ' + str(node_description[0]) + ' should be list')
                for link in node_description[3]:
                    if not isinstance(link, Link):
                        raise TypeError('a link information of ' + str(node_description[0]) + ' should be a Link class')
            self.__configurable_description = description

        def __get_configurable_description(self):
            """
            Get the configurable description
            :return: the network description (see description in the set function)
            :rtype: list of [int, Node.NodeType, list of int, list of Link]
            """
            return self.__configurable_description

        topology_type = property(__get_topology_type, __set_topology_type)      # Topology type we will use in the class
        # String with the information to build a non-cyclic topology
        non_cyclic_description = property(__get_non_cyclic_description, __set_non_cyclic_description)
        # String with the information to set the link information to the non-cyclic topology
        non_cyclic_link_description = property(__get_non_cyclic_link_description, __set_non_cyclic_link_description)
        # List with the complete information to create a configurable network
        configurable_description = property(__get_configurable_description, __set_configurable_description)

    class FrameType:
        """
        Parameters of a class of traffic. Percentage, deadline, period, size and end to end deadline
        """

        def __init__(self, percentage, period, deadline, size, starting_time, end_to_end):
            """
            Init the frame type
            :param percentage: frame type percentage
            :type percentage: float
            :param period: frame type period
            :type period: int
            :param deadline: frame type deadline
            :type deadline: int
            :param size: frame type size
            :type size: frame type
            :param starting_time: frame type starting time
            :type starting_time: int
            :param end_to_end: frame type end to end delay
            :type end_to_end: int
            """
            self.__set_percentage(percentage)
            self.__set_period(period)
            self.__set_deadline(deadline)
            self.__set_size(size)
            self.__set_starting_time(starting_time)
            self.__set_end_to_end(end_to_end)

        def __set_percentage(self, percentage):
            """
            Set the percentage of the frame type
            :param percentage: frame type percentage
            :type percentage: float
            """
            if not isinstance(percentage, float):
                raise TypeError('The percentage should be a float')
            if percentage < 0.0 or percentage > 1.0:
                raise ValueError('The percentage should be between 0.0 and 1.0')
            self.__percentage = percentage

        def __get_percentage(self):
            """
            Gets the percentage of the frame type
            :return: frame type percentage
            :rtype: float
            """
            return self.__percentage

        def __set_period(self, period):
            """
            Set the period of the frame type
            :param period: frame type period
            :type period: int
            """
            if not isinstance(period, int):
                raise TypeError('The period should be an integer')
            if period <= 0:
                raise ValueError('The period should be positive')
            self.__period = period

        def __get_period(self):
            """
            Gets the period of the frame type
            :return: frame type period
            :rtype: int
            """
            return self.__period

        def __set_deadline(self, deadline):
            """
            Set the deadline of the frame type
            :param deadline: frame type deadline
            :type deadline: int
            """
            if not isinstance(deadline, int):
                raise TypeError('The deadline should be an integer')
            if deadline <= 0 or deadline > self.__period:
                raise ValueError('The deadline should be positive and smaller than the period')
            self.__deadline = deadline

        def __get_deadline(self):
            """
            Gets the deadline of the frame type
            :return: frame type deadline
            :rtype: int
            """
            return self.__deadline

        def __set_size(self, size):
            """
            Set the size of the frame type
            :param size: frame type size
            :type size: int
            """
            if not isinstance(size, int):
                raise TypeError('The size should be an integer')
            if size <= 0:
                raise ValueError('The size should be positive')
            self.__size = size

        def __get_size(self):
            """
            Gets the size of the frame type
            :return: frame type size
            :rtype: int
            """
            return self.__size

        def __set_starting_time(self, starting_time):
            """
            Set the starting_time of the frame type
            :param starting_time: frame type starting_time
            :type starting_time: int
            """
            if not isinstance(starting_time, int):
                raise TypeError('The starting_time should be an integer')
            if starting_time < 0:
                raise ValueError('The starting_time should be positive')
            self.__starting_time = starting_time

        def __get_starting_time(self):
            """
            Gets the starting_time of the frame type
            :return: frame type starting_time
            :rtype: int
            """
            return self.__starting_time

        def __set_end_to_end(self, end_to_end):
            """
            Set the end_to_end of the frame type
            :param end_to_end: frame type end_to_end
            :type end_to_end: int
            """
            if not isinstance(end_to_end, int):
                raise TypeError('The end_to_end should be an integer')
            if end_to_end <= 0:
                raise ValueError('The end_to_end should be positive')
            self.__end_to_end = end_to_end

        def __get_end_to_end(self):
            """
            Gets the end_to_end of the frame type
            :return: frame type end_to_end
            :rtype: int
            """
            return self.__end_to_end

        percentage = property(__get_percentage, __set_percentage)
        period = property(__get_period, __set_period)
        deadline = property(__get_deadline, __set_deadline)
        size = property(__get_size, __set_size)
        starting_time = property(__get_starting_time, __set_starting_time)
        end_to_end = property(__get_end_to_end, __set_end_to_end)

    # Init and getter and setters #

    def __init__(self):
        """
        Initialization of the class
        """
        self.__graph = nx.DiGraph()     # Graph with the information of the topology created with the package NetworkX
        self.__switch_list = []         # List with the identifiers of the switches in the network
        self.__end_system_list = []     # List with the identifiers of the end systems in the network
        self.__link_list = []           # List with the identifiers of the links in the network in order as the config
        self.__link_objects_list = []   # List with the link objects matching the link list
        self.__frames = []              # List with the information of all frames
        self.__paths = []               # 3 dimension path matrix, [sender][receiver][path_number]
        self.__shortest_path = None     # If 1, only select shortest path, if 0, select all possible paths

    def __set_minimum_time_switch(self, minimum_time):
        """
        Set the minimum time a frame can stay in the switch in ns
        :param minimum_time: minimum time a frame can stay in the switch in ns
        :type minimum_time: int
        """
        if not isinstance(minimum_time, int):
            raise TypeError('The minimum time of a frame in the switch should be an integer')
        if minimum_time < 0:
            raise ValueError('The minimum time of a frame in the switch should be 0 or larger')
        self.__minimum_time_switch = minimum_time

    def __get_minimum_time_switch(self):
        """
        Get the minimum time a frame can stay in the switch in ns
        :return: minimum time a frame can stay in the switch in ns
        :rtype: int
        """
        return self.__minimum_time_switch

    def __set_self_healing_protocol(self, protocol):
        """
        Set the self-healing protocol
        :param protocol: self-healing protocol object
        :type protocol: SelfHealingProtocol
        """
        if not isinstance(protocol, self.SelfHealingProtocol):
            raise TypeError('The protocol should be a SelfHealingProtocol class')
        self.__self_healing_protocol = protocol

    def __get_self_healing_protocol(self):
        """
        Get the Self-Healing Protocol object
        :return: Self-Healing Protocol object
        :rtype: SelfHealingProtocol
        """
        return self.__self_healing_protocol

    def __set_num_frames(self, num_frames):
        """
        Set the number of frames in the network
        :param num_frames: number of frames
        :type num_frames: int
        """
        if not isinstance(num_frames, int):
            raise TypeError('The number of frames should be an integer')
        if num_frames <= 0:
            raise ValueError('The number of frames should be positive')
        self.__num_frames = num_frames

    def __get_num_frames(self):
        """
        Get the number of frames in the network
        :return: number of frames in the network
        :rtype: int
        """
        return self.__num_frames

    def __set_single(self, single):
        """
        Set the percentage of single frames
        :param single: percentage of single frames
        :type single:
        """
        if not isinstance(single, float):
            raise TypeError('The percentage of single frames should be a float')
        if single < 0.0 or single > 1.0:
            raise ValueError('The percentage of single frames should be between 0.0 and 1.0')
        self.__single = single

    def __get_single(self):
        """
        Get the percentage of single frames
        :return: percentage of single frames
        :rtype: float
        """
        return self.__single

    def __set_local(self, local):
        """
        Set the percentage of local frames
        :param local: percentage of local frames
        :type local: float
        """
        if not isinstance(local, float):
            raise TypeError('The percentage of local frames should be a float')
        if local < 0.0 or local > 1.0:
            raise ValueError('The percentage of local frames should be between 0.0 and 1.0')
        self.__local = local

    def __get_local(self):
        """
        Get the percentage of local frames
        :return: percentage of local frames
        :rtype: float
        """
        return self.__local

    def __set_multi(self, multi):
        """
        Set the percentage of multi frames
        :param multi: percentage of multi frames
        :type multi: float
        """
        if not isinstance(multi, float):
            raise TypeError('The percentage of multi frames should be a float')
        if multi < 0.0 or multi > 1.0:
            raise ValueError('The percentage of multi frames should be between 0.0 and 1.0')
        self.__multi = multi

    def __get_multi(self):
        """
        Get the percentage of multi frames
        :return: percentage of multi frames
        :rtype: float
        """
        return self.__multi

    def __set_broadcast(self, broadcast):
        """
        Set the percentage of broadcast frames
        :param broadcast: percentage of broadcast frames
        :type broadcast: float
        """
        if not isinstance(broadcast, float):
            raise TypeError('The percentage of broadcast frames should be a float')
        if broadcast < 0.0 or broadcast > 1.0:
            raise ValueError('The percentage of broadcast frames should be between 0.0 and 1.0')
        self.__broadcast = broadcast

    def __get_broadcast(self):
        """
        Get the percentage of broadcast frames
        :return: percentage of broadcast frames
        :rtype: float
        """
        return self.__broadcast

    def __set_topology(self, topology):
        """
        Creates and set the type of the topology we are going to use to describe the network
        :param topology: topology object
        :type topology: Network.Topology
        """
        if not isinstance(topology, Network.Topology):
            raise TypeError('The topology should be a Topology class')
        self.__topology = topology

    def __get_topology(self):
        """
        Get the topology of the network
        :return: network topology
        :rtype: Network.Topology
        """
        return self.__topology

    def __set_frame_type_list(self, frame_type_list):
        """
        Set the list of frame types to create the frames in the network
        :param frame_type_list: list of frame types classes with the information
        :type frame_type_list: list of FrameType
        """
        if not isinstance(frame_type_list, list):
            raise TypeError('The frame type should be a list')
        if not all(isinstance(frame_type, Network.FrameType) for frame_type in frame_type_list):
            raise TypeError('Elements in the frame type list are not from the FrameType class')
        self.__frame_type_list = frame_type_list

    def __get_frame_type_list(self):
        """
        Get the list of frame types to create the frames in the network
        :return: list of frame types classes with the information
        :rtype: list of FrameType
        """
        return self.__frame_type_list

    # Minimum time a frame has to stay into the switch in ns
    minimum_time_switch = property(__get_minimum_time_switch, __set_minimum_time_switch)
    # Self-Healing protocol parameters
    self_healing_protocol = property(__get_self_healing_protocol, __set_self_healing_protocol)
    num_frames = property(__get_num_frames, __set_num_frames)   # Number of frames in the network
    single = property(__get_single, __set_single)               # Percentage of frames with only one receiver
    # Percentage of frames where all receiver are at two hops of distance
    local = property(__get_local, __set_local)                  # Percentage of frames with a random number of receivers
    multi = property(__get_multi, __set_multi)                  # Percentage of frames with all nodes as receivers
    broadcast = property(__get_broadcast, __set_broadcast)
    # Topology object with the type and the information needed to construct the network
    topology = property(__get_topology, __set_topology)
    # List of FrameTypes to create the frames
    frame_type_list = property(__get_frame_type_list, __set_frame_type_list)

    # Auxiliary Functions #

    @staticmethod
    def __change_to_ns(value, time_unit):
        """
        Change the given value to nanoseconds
        :param value: time value
        :type value: int
        :param time_unit: time unit of the given value
        :type time_unit: TimeUnit
        :return: the converted time value into nanoseconds
        :rtype: int
        """
        if not isinstance(value, int):
            raise TypeError('The given value should be an integer')
        if not isinstance(time_unit, Network.TimeUnit):
            raise TypeError('The given time unit should be of the class TimeUnit')
        # Change to nanoseconds and return the value
        if time_unit == Network.TimeUnit.ns:
            return value
        if time_unit == Network.TimeUnit.us:
            return value * 1000
        if time_unit == Network.TimeUnit.ms:
            return value * 1000000
        if time_unit == Network.TimeUnit.s:
            return value * 1000000000

    @staticmethod
    def __change_to_mbs(value, speed_unit):
        """
        Change the given value to MB/s
        :param value: speed value
        :type value: int
        :param speed_unit: speed unit of the given value
        :type speed_unit: SpeedUnit
        :return: the converted speed value into MB/s
        :rtype: int
        """
        if not isinstance(value, int):
            raise TypeError('The given value should be an integer')
        if not isinstance(speed_unit, Network.SpeedUnit):
            raise TypeError('The given speed unit should be of the class SpeedUnit')
        # Change to MB/s and return the value
        if speed_unit == Network.SpeedUnit.kbs:
            return int(value/1000)
        if speed_unit == Network.SpeedUnit.mbs:
            return value
        if speed_unit == Network.SpeedUnit.gbs:
            return value * 1000

    # Network Functions #

    def create_network(self):
        """
        Create the network after set ALL the configuration variables needed. The parse function will do that from the
        xml file
        """
        # Start creating the topology, with all the nodes and links and its information
        self.__create_topology()
        if self.__shortest_path:
            self.__generate_all_shortest_paths()
        else:
            self.__generate_all_simple_paths()
        self.__generate_frames()

    def __add_switch(self):
        """
        Add into the NetworkX graph a new node with type => object.switch node, id => switch number.
        It also saves the identifier on the switch list
        """
        self.__switch_list.append(self.__graph.number_of_nodes())
        self.__graph.add_node(self.__graph.number_of_nodes(), type=Node(Node.NodeType.switch),
                              id=len(self.__switch_list) - 1)

    def __add_end_system(self):
        """
        Add into the NetworkX graph a new node with type => object.end_system node, id => node number
        It also saves the identifier on the end systems list
        """
        self.__end_system_list.append(self.__graph.number_of_nodes())
        self.__graph.add_node(self.__graph.number_of_nodes(), type=Node(Node.NodeType.end_system),
                              id=len(self.__end_system_list) - 1)

    def __change_switch_to_end_system(self, switch_id):
        """
        Change an already introduced switch to an end system. Just change it into NetworkX graph data manually
        :param switch_id: switch id
        :type switch_id: int
        """
        self.__graph.nodes[switch_id]['type'] = Node(Node.NodeType.end_system)
        self.__graph.nodes[switch_id]['id'] = len(self.__end_system_list)
        self.__end_system_list.append(switch_id)    # Remember to change also the lists of ids
        self.__switch_list.remove(switch_id)

    def __add_link(self, source, destination, link_type, speed):
        """
        Adds a bi-directional link, two directional links, between the source node and the destination node.
        It adds it into the NetworkX graph containing the object with the link information.
        As the links are not in order as the configuration file, we create and use a list of links ids and objects
        :param source: source node id
        :type source: int
        :param destination: destination node id
        :type destination: int
        :param link_type: link type
        :type link_type: Link.LinkType
        :param speed: link speed in MB/s
        :type speed: int
        """
        self.__graph.add_edge(source, destination, type=Link(speed, link_type), id=self.__graph.number_of_nodes() - 1)
        self.__link_list.append([source, destination])
        self.__link_objects_list.append(Link(speed, link_type))

    def __add_double_link_information(self, links, num_links, branch, parent_node):
        """
        Reads the link information and creates the links in both directions.
        Add the link type wired if is 'w' or wireless if is 'x'. # The speed is the rest of the string
        IMPORTANT: Note that we take the link id from num_links (as they are not sorted in the links array!)
        :param links: link description list
        :type links: list of str
        :param num_links: index of the number of links in the execution of the create topology
        :type num_links: int
        :param branch: branch to add the link
        :type branch: int
        :param parent_node: parent node to link the new created node
        :type parent_node: int
        """
        link_type = Link.LinkType.wired if links[num_links + branch][0] == 'w' else Link.LinkType.wireless
        speed = int(links[num_links + branch][1:])
        self.__add_link(parent_node, self.__graph.number_of_nodes() - 1, link_type, speed)
        self.__add_link(self.__graph.number_of_nodes() - 1, parent_node, link_type, speed)

    def __create_topology(self):
        """
        Given the topology class stored, creates the network nodes and links with its information. It will call to a
        different function to create the different possible topologies.
        We use NetworkX package to store the network information as it is easier to distribute all the information
        """
        # Check the topology type stored in the topology object and call the correspondent function
        if self.__topology.topology_type == self.Topology.TopologyType.non_cyclic:
            self.__create_non_cyclic_topology()
        elif self.__topology.topology_type == self.Topology.TopologyType.configurable:
            self.__create_configurable_topology()

    def __recursive_non_cyclic_topology(self, description, link_description, parent_node, num_calls, num_links):
        """
        Recursive function from non-cyclic topology that create all the nodes and links for a given bifurcation.
        It iterates the description list by the number of call to create switches, end systems or change switch to
        end system.
        It also uses the link description to create the links needed to connect all the nodes. It uses the num_links
        to remember in which link we are at the current call to correctly map each link to its nodes
        :param description: network description already parsed into integers
        :type description: list of int
        :param link_description: network link description, still as str with the link type and speed
        :type link_description: list of str
        :param parent_node: id of the parent node of the current call
        :type parent_node: int
        :param num_calls: number of calls that has been done to this function. It is used to know which description
        element we need to take care
        :type num_calls: int
        :param num_links: number of links that has been done to the function .It is needed to map the links in the
        correct order as they are not correctly ordered in the links array for the perspective of the recursive
        function)
        :type num_links: int
        :return: the updated number of calls and the number of links to track correctly during the recursion
        :rtype: [int, int]
        """
        try:    # We try to catch wrongly formulated descriptions
            # Create new leafs and end systems and link to them to the parent node if the description is negative
            if description[num_calls] < 0:
                # For all the leafs, we add an end system and link it to the parent switch with two links
                for leaf in range(abs(description[num_calls])):
                    self.__add_end_system()
                    self.__add_double_link_information(link_description, num_links, leaf, parent_node)
                    # Also opposite direction
                # Return subtracting the last links created by the last branch from the number of links, we want the
                # number of links when the branch is starting, no after (due to recursion)
                return num_links - int(description[num_calls]), num_calls

            # Finished branch, change switch parent node into an end system as it will not be continued
            if description[num_calls] == 0:
                self.__change_switch_to_end_system(parent_node)
                return num_links, num_calls     # Return the same values as we did not move into the branch

            # Create a new branch with switches if the description is positive
            if description[num_calls] > 0:
                it_links = num_calls    # Save the starting number of links as it will be lost while we iterate
                last_call_link = 0  # Create variable to save the last link that was call (to return when we finish)

                for branch in range(description[num_calls]):    # For all the new branches, create a switch and link it
                    new_parent = self.__graph.number_of_nodes()
                    self.__add_switch()
                    self.__add_double_link_information(link_description, num_links, branch, parent_node)

                    # Check which link is calling, if last call is bigger, set it to last call
                    if last_call_link > it_links + (description[num_calls] - branch):
                        links_to_call = last_call_link
                    else:
                        links_to_call = it_links + (description[num_calls])
                    # Call the recursive for the new branch, we save the last call link to recover it when we return
                    # after the branch created by this recurse call is finished
                    last_call_link, num_calls = self.__recursive_non_cyclic_topology(description, link_description,
                                                                                     new_parent, num_calls + 1,
                                                                                     links_to_call)
                return last_call_link, num_calls    # Return when all the branches have been created and closed

        except IndexError:
            raise ValueError('The network description is wrongly formulated, there are open branches')

    def __create_non_cyclic_topology(self):
        """
        Create a non-cyclic network with the information in the description and link strings.
        It will call a recursive function where each bifurcation will be called and its links connected.
        It starts with the 'center' switch and expands creating switches until it arrives to a negative or 0
        bifurcation, that means it is the end and it creates an end system.
        The links are bi-directional, so we create two different directional links to represent this.
        """
        # Prepare the description strings into lists for easier indexing
        if not isinstance(self.__topology.non_cyclic_description, str):
            raise TypeError('The description string for non-cyclic topologies is not configured')
        if not isinstance(self.__topology.non_cyclic_link_description, str):
            raise TypeError('The link description string for non-cyclic topologies is not configured')

        description_list = self.__topology.non_cyclic_description.split(';')
        description_list = [int(numeric_string) for numeric_string in description_list]
        links_list = self.__topology.non_cyclic_link_description.split(';')

        # Start the center switch as the parent switch with id 0
        self.__add_switch()

        # Num links and num calls are auxiliary variables to map the order in which the links are created and to check
        # if the creation of the network was successful
        num_links, num_calls = self.__recursive_non_cyclic_topology(description_list, links_list, 0, 0, 0)

        # Check if there are additional elements that should not be in the network
        if num_calls != len(description_list) - 1:
            raise ValueError('The network description for non-cyclic networks is wrongly formulated')

    def __create_configurable_topology(self):
        """
        Creates a network topology with the information of all the nodes, its connections and the link specifications
        of each connection. First create all nodes and later create the connections
        """
        for node in self.__topology.configurable_description:       # First create all nodes
            if node[1] == Node.NodeType.switch:
                self.__add_switch()
            elif node[1] == Node.NodeType.end_system:
                self.__add_end_system()

        for node_id, node in enumerate(self.__topology.configurable_description):   # Then create all connections
            for num_connection, connection in enumerate(node[2]):
                link = node[3][num_connection]
                self.__add_link(node_id, connection, link.link_type, link.speed)

    def __generate_all_shortest_paths(self):
        """
        Generate all the shortest paths from every end system to every other end system
        """
        for i in range(self.__graph.number_of_nodes()):
            self.__paths.append([])
            for j in range(self.__graph.number_of_nodes()):
                self.__paths[i].append([])

        # Iterate over all possible end systems sender and possible end systems receivers
        for sender_id in self.__end_system_list:
            for receiver_id in self.__end_system_list:
                if sender_id != receiver_id:        # If not the same, find the path
                    # Get the shortest path
                    node_path = nx.shortest_path(self.__graph, sender_id, receiver_id)
                    first_iteration = False
                    previous_node = None
                    path = []
                    for node in node_path:  # Convert the list of nodes to list of links
                        if not first_iteration:
                            first_iteration = True
                        else:  # Find the index in the link list
                            path.append(self.__link_list.index([previous_node, node]))
                        previous_node = node
                    self.__paths[sender_id][receiver_id].append(path)

    def __generate_all_simple_paths(self):
        """
        Generate all the simple paths (no cycles) from every end system to every other end system
        Fills the 3 dimension path matrix, first dimension is the sender, second dimension is the receiver, third
        dimension is a path number that contain the list of links to arrive from the sender to the receiver
        """
        # Init the path 3-dimension matrix with empty arrays
        for i in range(self.__graph.number_of_nodes()):
            self.__paths.append([])
            for j in range(self.__graph.number_of_nodes()):
                self.__paths[i].append([])

        # Iterate over all possible end systems sender and possible end systems receivers
        for sender_id in self.__end_system_list:
            for receiver_id in self.__end_system_list:
                if sender_id != receiver_id:        # If not the same, find the path
                    # Get all the possible paths
                    for path_id, node_path in enumerate(nx.all_simple_paths(self.__graph, sender_id, receiver_id)):
                        first_iteration = False
                        previous_node = None
                        path = []
                        for node in node_path:      # Convert the list of nodes to list of links
                            if not first_iteration:
                                first_iteration = True
                            else:   # Find the index in the link list
                                path.append(self.__link_list.index([previous_node, node]))
                            previous_node = node
                        self.__paths[sender_id][receiver_id].append(path)

    def __generate_frames(self):
        """
        Generate frames for the network. You can choose the number of frames and the percentage of every type.
        Once a frame has decided the sender and receivers, see which type of frames will be
        """
        # For every frame, we calculate the probability to have different receivers configuration, we choose a sender
        # randomly, and then, depending of the configuration, choose randomly the receivers. Always end systems
        for frame in range(self.__num_frames):
            percentage = random()
            receivers = []
            sender = choice(self.__end_system_list)     # Select the sender end system

            if percentage < self.__broadcast:           # Broadcast
                receivers = list(self.__end_system_list)
                receivers.remove(sender)                # The sender cannot be a receiver

            if percentage < self.__broadcast + self.__single:   # Single
                receivers = list(self.__end_system_list)
                receivers.remove(sender)
                receivers = [choice(receivers)]         # Choose one only, receivers is a list always, even of 1 element

            if percentage < self.__broadcast + self.__single + self.__multi:    # Multi
                receivers = list(self.__end_system_list)
                receivers.remove(sender)
                shuffle(receivers)                          # Shuffle to select a random number easily
                num_receivers = randint(1, len(receivers))
                receivers = receivers[0:num_receivers]      # Easy way to select a random number of receivers

            else:   # Locally, we calculate it, taking all connections from the sender and go to the next one
                for first_hop in self.__link_list:
                    receivers = []
                    if first_hop[0] == sender:       # If the sender of the link is the same as the frame sender
                        for second_hop in self.__link_list:
                            # If the second hope for the same node is an end system
                            if second_hop[0] == first_hop[1] and second_hop[1] in self.__end_system_list:
                                receivers.append(second_hop[1])
                if not receivers:                 # If there is not local end systems, we just find a random receiver
                    receivers = list(self.__end_system_list)
                    receivers.remove(sender)
                    receivers = [choice(receivers)]

            # Select the frame type of the current frame
            percentage = random()
            accumulative_percentage = 0.0
            for type_id, frame_type in enumerate(self.__frame_type_list):      # Choose one frame type
                if percentage < frame_type.percentage + accumulative_percentage:
                    self.__frames.append(Frame(sender, receivers, frame_type.period, frame_type.deadline,
                                               frame_type.size, frame_type.starting_time, frame_type.end_to_end))
                    break       # we stop the for after everything
                else:
                    accumulative_percentage += frame_type.percentage       # If not, we go for the next one

    # Input Functions #

    def read_network_configuration_xml(self, configuration_file):
        """
        Reads the parameters from the given configuration file and saves it into the internal memory of the class for
        other functions being able to create the network.
        We start reading the basic information of the network configuration.
        Then we read the network topology
        :param configuration_file: name and relative path of the configuration file
        :type configuration_file: str
        """
        # Open the file if exists
        try:
            tree = Xml.parse(configuration_file)
        except Xml.ParseError:
            raise Exception('Could not read the network configuration file')
        root_xml = tree.getroot()       # Get the root of the xml file

        self.__read_basic_network_information_xml(root_xml)     # Get the basic information
        self.__read_topology_description_xml(root_xml)          # Get the topology description
        self.__read_traffic_basic_information_xml(root_xml)     # Get the basic traffic information
        self.__read_traffic_types_xml(root_xml)                 # Get the frame types information

    def __read_basic_network_information_xml(self, root):
        """
        Read the basic information configuration of the xml file
        :param root: root of the xml file
        :type root: xml.etree.ElementTree.Element
        """
        try:
            basic_information = root.find('Basic_Information')      # Position into the basic information branch
        except Xml.ParseError:
            raise Exception('Could not find the network basic information')
        self.__set_minimum_time_switch(self.__read_switch_information_xml(basic_information))
        self.__set_self_healing_protocol(self.__read_self_healing_information_xml(basic_information))
        self.__shortest_path = int(basic_information.find('Shortest_Path').text)

    def __read_switch_information_xml(self, basic_information):
        """
        Given the branch with the basic information, returns all the switch information, in this case,
        the minimum time a frame needs to stay in the frame
        :param basic_information: xml branch with the basic information
        :type basic_information: xml.etree.ElementTree.Element
        :return: the minimum time a frame can stay in a switch
        :rtype: int
        """
        try:
            minimum_time = basic_information.find('Switch_Information/Minimum_Time')
        except Xml.ParseError:
            raise Exception('Could not find the switch minimum time information')
        return self.__change_to_ns(int(minimum_time.text), self.TimeUnit.from_string(minimum_time.attrib['unit']))

    def __read_self_healing_information_xml(self, basic_information):
        """
        Given the branch with the basic information, returns two tuples with the protocol period and time with their
        time units
        :param basic_information: xml branch with the basic information
        :type basic_information: xml.etree.ElementTree.Element
        :return: the self-healing protocol information
        :rtype: SelfHealingProtocol
        """
        try:
            period = basic_information.find('Self-Healing_Protocol/Period')
        except Xml.ParseError:
            raise Exception('Could not find the self-healing protocol period information')
        try:
            time = basic_information.find('Self-Healing_Protocol/Time')
        except Xml.ParseError:
            raise Exception('Could not find the self-healing protocol time information')
        return self.SelfHealingProtocol(self.__change_to_ns(int(period.text),
                                                            self.TimeUnit.from_string(period.attrib['unit'])),
                                        self.__change_to_ns(int(time.text),
                                                            self.TimeUnit.from_string(time.attrib['unit'])))

    def __read_topology_description_xml(self, root):
        """
        Read the topology description of the network.
        It is capable of reading different ways to describe the topology, as much as types as in TopologyType class
        :param root: root of the xml file
        :type root: xml.etree.ElementTree.Element
        """
        try:
            topology_description = root.find('Topology_Description')
        except Xml.ParseError:
            raise Exception('Could not find the network topology')
        # Get the topology type and init the topology object
        topology_type = self.Topology.TopologyType.from_string(topology_description.attrib['type'])
        self.__set_topology(self.Topology(topology_type))
        if topology_type == self.Topology.TopologyType.non_cyclic:  # If non-cyclic, read and save it into the topology
            description, link_description = self.__read_non_cyclic_topology_description_xml(topology_description)
            self.__topology.non_cyclic_description = description
            self.__topology.non_cyclic_link_description = link_description
        elif topology_type == self.Topology.TopologyType.configurable:  # If configurable topology, read and save it
            self.__topology.configurable_description = \
                self.__read_configurable_topology_description_xml(topology_description)

    def __read_non_cyclic_topology_description_xml(self, topology_description):
        """
        Read the non-cyclic topology description from the xml file
        :param topology_description: position branch where the information of the topology description is
        :type topology_description: Xml.etree.ElementTree.Element
        :return: string with the network description in pozo network language and the link description
        :rtype: str, str
        """
        try:        # Find all the bifurcations with the topology information and links information
            bifurcation_list = topology_description.findall('Bifurcation')
        except Xml.ParseError:
            raise Exception('No bifurcations are defined in the topology description')

        # Initiate the strings that will contain the topology and links information
        topology_str = ''
        links_str = ''

        # For all the bifurcations in the topology description, read the number of links and its information
        for bifurcation in bifurcation_list:
            try:    # Try to see if there is the number of links and add it to the topology description link
                topology_str += bifurcation.find('NumberLinks').text + ';'
                num_links = abs(int(bifurcation.find('NumberLinks').text))   # Number of links in the bifurcation
            except Xml.ParseError:
                raise Exception('A bifurcation does not have the number of links')

            # Extract also the type of link and its speed
            try:    # Try to see if there every link has its description
                link_list = bifurcation.findall('Link')
            except Xml.ParseError:
                raise Exception('A bifurcation has no link descriptions')
            if num_links != len(link_list):
                raise Exception('A bifurcation has not the same link description as stated in the NumberLinks')
            for link in link_list:
                # Extract the link type
                link_type = Link.LinkType.from_string(link.attrib['category'])
                if link_type == Link.LinkType.wired:
                    links_str += 'w'    # We state is a wired link
                elif link_type == Link.LinkType.wireless:
                    links_str += 'x'    # We state is a wireless link
                else:
                    raise Exception('A link has not a value in the LinkType class')
                # Extract the link speed
                try:
                    speed = link.find('Speed')
                except Xml.ParseError:
                    raise Exception('The link in the bifurcation should have defined the speed')
                speed_type = Network.SpeedUnit.from_string(speed.attrib['unit'])
                speed = self.__change_to_mbs(int(speed.text), speed_type)
                links_str += str(speed) + ';'

        return topology_str[0:-1], links_str[0:-1]  # Return both strings, we remove the last ';'

    def __read_configurable_topology_description_xml(self, topology_description):
        """
        Read the complete topology description that states all the nodes and its connections, together with the link
        information.
        :param topology_description: position branch where the information of the topology description is
        :type topology_description: Xml.etree.ElementTree.Element
        :return: a list of tuple with index equal to the number of nodes in the network. The tuple contains 3 elements:
        the node id, the type of node, a list of the nodes id that the index of the main tuple is connected, and a list
        of the information of the link for each of the previous connection
        :rtype: list of [int, Network.Node.NodeType, list of int, list of Network.Link]
        """
        try:
            node_list = topology_description.findall('Node')
        except Xml.ParseError:
            raise Exception('No nodes are defined in the topology description')

        network_description = []    # Initiate the list that will contain the topology description

        for node in node_list:      # Read all the nodes
            network_description.append([None, None, [], []])      # We found a node, init the list with its information

            try:
                network_description[-1][0] = int(node.find('NodeID').text)
            except Xml.ParseError:
                raise Exception('No NodeID found')

            network_description[-1][1] = Node.NodeType.from_string(node.attrib['category'])

            try:
                connection_list = node.findall('Connection')
            except Xml.ParseError:
                raise Exception('No connections found in a node')

            for connection in connection_list:      # For all the connections
                try:    # Try to find the node id of the current node connection
                    network_description[-1][2].append(int(connection.find('NodeID').text))
                    link = connection.find('Link')
                    link_speed = link.find('Speed')
                    link_speed = self.__change_to_mbs(int(link_speed.text),
                                                      Network.SpeedUnit.from_string(link_speed.attrib['unit']))
                    link_type = Link.LinkType.from_string(link.attrib['category'])
                    network_description[-1][3].append(Link(link_speed, link_type))
                except Xml.ParseError:
                    raise Exception('The current node connection does not have the node id and link information')
        return network_description

    def __read_traffic_basic_information_xml(self, topology_description):
        """
        Read the basic information of the traffic configuration for the network, which includes the number of frames
        and the percentage of frames that will be single, local, multi and broadcast. A single has only one node
        receiver, a local is sent to all end systems that are two hops away, multi sends to a random number of
        nodes, and broadcast to all nodes.
        :param topology_description: position branch where the information of the topology description is
        :type topology_description: Xml.etree.ElementTree.Element
        """
        try:        # Find the traffic basic information branch
            traffic_basic_xml = topology_description.find('Traffic_Description/Traffic_Information')
        except Xml.ParseError:
            raise Exception('No traffic basic information branch found')

        try:        # Find all the values of the basic information branch
            self.__num_frames = int(traffic_basic_xml.find('Number_Frames').text)
            self.__single = float(traffic_basic_xml.find('Single').text)
            self.__local = float(traffic_basic_xml.find('Local').text)
            self.__multi = float(traffic_basic_xml.find('Multi').text)
            self.__broadcast = float(traffic_basic_xml.find('Broadcast').text)
            if self.__single + self.__local + self.__multi + self.__broadcast != 1.0:
                raise ValueError('The sum of frame percentages must be 1.0 in the traffic basic information')
        except Xml.ParseError:
            raise Exception('There is missing traffic basic information')

    def __read_traffic_types_xml(self, topology_description):
        """
        Read the different traffic types the frames will follow. They have information of period, deadline, size,
        end to end deadline and the percentage of frames that follow that specification.
        There is a list of different specifications that must add 1.0 at the end.
        :param topology_description: position branch where the information of the topology description is
        :type topology_description: Xml.etree.ElementTree.Element
        """
        try:        # Find the traffic basic information branch
            frame_type_list_xml = topology_description.findall('Traffic_Description/Frame_Description/Frame_Type')
        except Xml.ParseError:
            raise Exception('No traffic types information branches found')

        frame_type_list = []

        for frame_type_id, frame_type_xml in enumerate(frame_type_list_xml):    # Take the information of a frame type
            try:
                percentage = float(frame_type_xml.find('Percentage').text)
                size = int(frame_type_xml.find('Size').text)
                period = frame_type_xml.find('Period')
                period = self.__change_to_ns(int(period.text), self.TimeUnit.from_string(period.attrib['unit']))
                deadline = frame_type_xml.find('Deadline')
                deadline = self.__change_to_ns(int(deadline.text), self.TimeUnit.from_string(deadline.attrib['unit']))
                starting_time = frame_type_xml.find('StartingTime')
                starting_time = self.__change_to_ns(int(starting_time.text),
                                                    self.TimeUnit.from_string(starting_time.attrib['unit']))
                end_to_end = frame_type_xml.find('EndToEnd')
                end_to_end = self.__change_to_ns(int(end_to_end.text),
                                                 self.TimeUnit.from_string(end_to_end.attrib['unit']))
            except Xml.ParseError:
                raise Exception('The frame type ' + str(frame_type_id) + ' is not well defined')

            frame_type_list.append(self.FrameType(percentage, period, deadline, size, starting_time, end_to_end))

        # Check if the percentages match
        accumulative_percentage = 0.0
        for frame_type in frame_type_list:
            accumulative_percentage += frame_type.percentage
        if accumulative_percentage != 1.0:
            raise ValueError('The sum of percentages should be 1.0')

        # Save the frame list into the object
        self.__frame_type_list = frame_type_list

    # Output Functions #

    def write_network_xml(self, name):
        """
        Writes all the network information needed in a XML file by the given name
        :param name: path and name of the xml file to create, if it already exists, it deletes the file and re-write it
        :type name: str
        """
        if not isinstance(name, str):
            raise TypeError('The name should be a string')

        network_xml = Xml.Element('Network')        # Top of the XML tree structure

        self.__write_general_information_xml(network_xml)
        self.__write_topology_information_xml(network_xml)
        self.__write_frames_xml(network_xml)

        # Write the final file
        output_xml = minidom.parseString(Xml.tostring(network_xml)).toprettyxml(indent="   ")
        with open(name, "w") as f:
            f.write(output_xml)

    def __write_general_information_xml(self, top_xml):
        """
        Writes the general network general information on the top of the given XML structure
        :param top_xml: top branch of the XML structure
        :type top_xml: Xml.Element
        """
        if not isinstance(top_xml, Xml.Element):
            raise TypeError('The top_xml should be an XML.Element object')

        general_information_xml = Xml.SubElement(top_xml, 'General_Information')

        Xml.SubElement(general_information_xml, 'Number_Frames').text = str(len(self.__frames))
        Xml.SubElement(general_information_xml, 'Number_Switches').text = str(len(self.__switch_list))
        Xml.SubElement(general_information_xml, 'Number_End_Systems').text = str(len(self.__end_system_list))
        Xml.SubElement(general_information_xml, 'Number_Links').text = str(len(self.__link_list))

        switch_information_xml = Xml.SubElement(general_information_xml, 'Switch_Information')
        Xml.SubElement(switch_information_xml, 'Minimum_Time').text = str(self.__minimum_time_switch)

        self_healing_information_xml = Xml.SubElement(general_information_xml, 'Self-Healing_Protocol')
        Xml.SubElement(self_healing_information_xml, 'Period').text = str(self.__self_healing_protocol.period)
        Xml.SubElement(self_healing_information_xml, 'Time').text = str(self.__self_healing_protocol.time)

    def __write_topology_information_xml(self, top_xml):
        """
        Writes the network topology information on the top of the XML given XML structure
        :param top_xml: top branch of the XML structure
        :type top_xml: Xml.Element
        """
        if not isinstance(top_xml, Xml.Element):
            raise TypeError('The top_xml should be an XML.Element object')

        topology_xml = Xml.SubElement(top_xml, 'Topology')

        node_top_xml = Xml.SubElement(topology_xml, 'Nodes')
        for node_id, node in self.__graph.nodes(data=True):       # For all the nodes in the NetworkX graph
            node_xml = Xml.SubElement(node_top_xml, 'Node')
            node_xml.set('category', node['type'].node_type_to_string())    # Node type
            Xml.SubElement(node_xml, 'NodeID').text = str(node_id)          # Node ID

            # Add all the current node out connections
            out_connections_xml = Xml.SubElement(node_xml, 'Out_Connections')
            for link_id, link in enumerate(self.__link_list):
                if link[0] == node_id:      # If the node is sending information through that link
                    connection_xml = Xml.SubElement(out_connections_xml, 'Connection')
                    Xml.SubElement(connection_xml, 'NodeID').text = str(link[1])
                    Xml.SubElement(connection_xml, 'LinkID').text = str(link_id)

            # Add all the current node in connections
            in_connections_xml = Xml.SubElement(node_xml, 'In_Connections')
            for link_id, link in enumerate(self.__link_list):
                if link[1] == node_id:  # If the node is receiving information through that link
                    connection_xml = Xml.SubElement(in_connections_xml, 'Connection')
                    Xml.SubElement(connection_xml, 'NodeID').text = str(link[0])
                    Xml.SubElement(connection_xml, 'LinkID').text = str(link_id)

        link_top_xml = Xml.SubElement(topology_xml, 'Links')
        for link_id, link in enumerate(self.__link_objects_list):       # For all the links in the network
            link_xml = Xml.SubElement(link_top_xml, 'Link')
            link_xml.set('category', str(link.link_type))
            Xml.SubElement(link_xml, 'LinkID').text = str(link_id)
            Xml.SubElement(link_xml, 'Speed').text = str(link.speed)
            Xml.SubElement(link_xml, 'Node_Source').text = str(self.__link_list[link_id][0])
            Xml.SubElement(link_xml, 'Node_Destination').text = str(self.__link_list[link_id][1])

        self.__write_paths_xml(topology_xml)        # Do not forget to write all the possible paths also

    def __write_paths_xml(self, topology_xml):
        """
        Write all the possible paths from a end system to any other end system (sender to receiver)
        :param topology_xml: topology branch of the XML structure
        :type topology_xml: Xml.SubElement
        """
        paths_xml = Xml.SubElement(topology_xml, 'Paths')
        for sender_id in self.__end_system_list:

            sender_xml = Xml.SubElement(paths_xml, 'Sender')
            Xml.SubElement(sender_xml, 'SenderID').text = str(sender_id)

            receiver_list_xml = Xml.SubElement(sender_xml, 'Receivers')
            for receiver_id in self.__end_system_list:      # For all possible combinations
                if sender_id != receiver_id:
                    receiver_xml = Xml.SubElement(receiver_list_xml, 'Receiver')
                    Xml.SubElement(receiver_xml, 'ReceiverID').text = str(receiver_id)
                    path_list_xml = Xml.SubElement(receiver_xml, 'Paths')
                    for path in self.__paths[sender_id][receiver_id]:   # For all the paths, write them out
                        path_str = ''
                        for link in path:
                            path_str += str(link) + ';'
                        Xml.SubElement(path_list_xml, 'Path').text = path_str[:-1]

    def __write_frames_xml(self, top_xml):
        """
        Write the information of all frames in top branch of the XML structure
        :param top_xml: top branch of the XML structure
        :type top_xml: Xml.Element
        """
        frame_list_xml = Xml.SubElement(top_xml, 'Frames')
        for frame_id, frame in enumerate(self.__frames):
            frame_xml = Xml.SubElement(frame_list_xml, 'Frame')
            Xml.SubElement(frame_xml, 'FrameID').text = str(frame_id)
            Xml.SubElement(frame_xml, 'Period').text = str(frame.period)
            Xml.SubElement(frame_xml, 'Deadline').text = str(frame.deadline)
            Xml.SubElement(frame_xml, 'Size').text = str(frame.size)
            Xml.SubElement(frame_xml, 'StartingTime').text = str(frame.starting_time)
            Xml.SubElement(frame_xml, 'EndToEnd').text = str(frame.end_to_end_delay)
            Xml.SubElement(frame_xml, 'SenderID').text = str(frame.sender_id)
            receiver_str = ''
            for receiver_id in frame.receivers_id:
                receiver_str += str(receiver_id) + ';'
            Xml.SubElement(frame_xml, 'ReceiversID').text = receiver_str[:-1]
