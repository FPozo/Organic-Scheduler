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


class Node:
    """
    Class with the information of a node in the network
    """

    class NodeType(Enum):
        """
        Class to define different node types
        """
        switch = 1
        end_system = 2
        access_point = 3

        @staticmethod
        def from_string(string):
            """
            From a given string, return the node type
            :param string: node type in string format
            :type string: str
            :return: node type
            :rtype: Node.NodeType
            """
            if not isinstance(string, str):
                raise TypeError('The string should be a string')
            if string == 'switch':
                return Node.NodeType.switch
            if string == 'end_system':
                return Node.NodeType.end_system
            if string == 'access_point':
                return Node.NodeType.access_point
            raise ValueError('The string should be in the NodeType class')

    def __init__(self, node_type):
        """
        Init the class with the node_type
        :param node_type: node type
        :type node_type: NodeType
        """
        self.__set_type(node_type)

    def __str__(self):
        """
        Returns a string with the information of the class
        :return: information of the class
        :rtype: str
        """
        if self.__node_type is self.NodeType.end_system:
            return 'The node is an End System'
        elif self.__node_type is self.NodeType.switch:
            return 'The node is a Switch'
        else:
            return 'The node is an Access Point'

    def __set_type(self, node_type):
        """
        Set the type of the node
        :param node_type: node type
        :type node_type: NodeType
        """
        if node_type not in self.NodeType:
            raise TypeError('Only End Systems, Switches and Access Points supported now, check NodeType')
        self.__node_type = node_type

    def __get_type(self):
        """
        Get the type of the node
        :return: node type
        :rtype: NodeType
        """
        return self.__node_type

    def node_type_to_string(self):
        """
        Returns the string representation of the node type
        :return: string representation of the node type
        :rtype: str
        """
        if self.__node_type == Node.NodeType.switch:
            return 'switch'
        if self.__node_type == Node.NodeType.end_system:
            return 'end_system'
        if self.__node_type == Node.NodeType.access_point:
            return 'access_point'

    node_type = property(__get_type, __set_type)
