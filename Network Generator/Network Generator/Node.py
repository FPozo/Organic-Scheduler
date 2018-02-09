# -*- coding: utf-8 -*-

"""* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  Node Class                                                                                                         *
 *  Network Generator                                                                                                  *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 09/02/18.                                                                             *
 *  Copyright © 2018 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 *  Class for the nodes in the network. Basic class that only defines node type (switch, end system or access point).  *
 *  It may be interesting to add new information such as memory of switches or more types in future work               *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * """

from enum import Enum


class NodeType(Enum):
    """
    Class to define different node types
    """
    switch = 1
    end_system = 2
    access_point = 3


class Node:
    """
    Class with the information of a node in the network
    """

    def __init__(self, node_type):
        """
        Init the class with the node_type
        :param node_type: node type
        :type node_type: NodeType
        """
        self.__set_type(node_type)

    def __set_type(self, node_type):
        """
        Set the type of the node
        :param node_type: node type
        :type node_type: NodeType
        """
        if node_type not in NodeType:
            raise TypeError('Only End Systems, Switches and Access Points supported now, check NodeType')
        self.__node_type = node_type

    def __get_type(self):
        """
        Get the type of the node
        :return: node type
        :rtype: NodeType
        """
        return self.__node_type

    node_type = property(__get_type, __set_type)
