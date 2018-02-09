# -*- coding: utf-8 -*-

"""* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  Link Class                                                                                                         *
 *  Network Generator                                                                                                  *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 08/02/18.                                                                             *
 *  Copyright © 2018 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 *  Class for the links in the network. It contains the speed and the type (wired or wireless) information of the link *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * """

from enum import Enum


class LinkType(Enum):
    """
    Class to define the link types
    """
    wired = 1
    wireless = 2


class Link:
    """
    Class that contains the information of the link (speed and type)
    """

    def __init__(self, speed=100, link_type=LinkType.wired):
        """
        Initialization of the link, if no values are given we suppose a standard wired link with speed of 100 MB/s
        :param speed: Speed of the link in MB/s
        :type speed: int
        :param link_type: Type of the link (wired or wireless)
        :type link_type: LinkType
        """
        self.__set_speed(speed)             # Link speed in MB/s
        self.__set_type(link_type)          # Link type (wired or wireless)

    def __str__(self):
        """
        String to transform the class to string
        :return: the link information string
        :rtype: str
        """
        # Check the type of the link
        if self.__link_type == LinkType.wired:
            return "Wired link with speed " + str(self.__speed) + " MB/s"
        else:
            return "Wireless link with speed " + str(self.__speed) + " MB/s"

    def __get_speed(self):
        """
        Get the speed of the link in MB/s
        :return: speed of the link in MB/s
        :rtype: int
        """
        return self.__speed

    def __set_speed(self, speed):
        """
        Set the speed of the link in MB/s
        :param speed: Link speed in MB/s
        :type speed: int
        """
        if type(speed) is not int or float:
            raise TypeError('The speed should be a number')
        if speed <= 0:
            raise ValueError('The speed should be positive')
        self.__speed = speed

    def __get_type(self):
        """
        Get the type of the link
        :return: type of the link
        :rtype: LinkType
        """
        return self.__link_type

    def __set_type(self, link_type):
        """
        Set the link type (wired or wireless)
        :param link_type: link type
        :type link_type: LinkType
        """
        if link_type not in LinkType:
            raise TypeError('Only wired and wireless link supported now, check LinkType')
        self.__link_type = link_type

    speed = property(__get_speed, __set_speed)      # Link speed in MB/s
    link_type = property(__get_type, __set_type)    # Link type (wired or wireless)
