# -*- coding: utf-8 -*-

"""* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  Frame Class                                                                                                        *
 *  Network Generator                                                                                                  *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 09/02/18.                                                                             *
 *  Copyright © 2018 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 *  Class for the frames in the network. A frame has only one node (end system) sender and one or multiple (also end   *
 *  systems) receivers. All frames must have a period in nanoseconds and a in bytes in the range of the Ethernet       *
 *  Standard frames. They may also have a deadline (also in nanoseconds).                                              *
 *  A frame is possible to have and end-to-end delay from the sender to the receiver (different for any if needed).    *
 *  It is possible for a frame to need to start after an specific amount of time since the schedule started.           *
 *  Frames contain a path of links to arrive from the sender to the receiver, if it is none, it means it has not       *
 *  decided yet, and other entity (such as a network or a scheduler) will decide it later.                             *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * """


class Frame:
    """
    Class with the information of a time-triggered frame
    """

    def __init__(self, sender_id, receivers_id, period, deadline=0, size=1000, starting_time=0, end_to_end=None):
        """
        Initialization of a time-triggered frame
        :param sender_id: sender id
        :type sender_id: int
        :param receivers_id: list of receivers id
        :type receivers_id: list of int
        :param period: period in ns
        :type period: int
        :param deadline: frame deadline in ns
        :type deadline: int
        :param size: frame size, we follow the Ethernet Standard, so, the payload can be between 64 and 1500 bytes
        :type size: int
        :param starting_time: starting time in ns
        :type starting_time: int
        :param end_to_end: end to end delays of the frame in ns
        :type end_to_end: Union[int, list of int, None]
        """
        self.__set_sender_id(sender_id)
        self.__set_receivers_id(receivers_id)
        self.__set_period(period)
        self.__set_deadline(deadline)
        self.__set_size(size)
        self.__set_starting_time(starting_time)
        self.__set_end_to_end_delay(end_to_end)

    def __str__(self):
        """
        Transform the information of the time-triggered frame to a string
        :return: a string with the time-triggered frame information
        :rtype: str
        """
        return_text = "Time-Triggered Frame information =>\n"
        return_text += "    Sender id     : " + str(self.__sender_id) + "\n"
        return_text += "    Receivers ids : " + str(self.__receivers_id) + "\n"
        return_text += "    Path          : " + str(self.__paths) + "\n"
        return_text += "    End_to_End    : " + str(self.__end_to_end_delay) + " nanoseconds\n"
        return_text += "    Period        : " + str(self.__period) + " nanoseconds\n"
        return_text += "    Starting      : " + str(self.__starting_time) + " nanoseconds\n"
        return_text += "    Deadline      : " + str(self.__deadline) + " nanoseconds\n"
        return_text += "    Size          : " + str(self.__size) + " bytes"
        return return_text

    def __set_sender_id(self, sender_id):
        """
        Set the sender id of the frame
        :param sender_id: sender id
        :type sender_id: int
        """
        if not isinstance(sender_id, int):
            raise TypeError('It has to be an integer identifier')
        if sender_id < 0:
            raise ValueError('There are not negative identifiers')
        self.__sender_id = sender_id

    def __get_sender_id(self):
        """
        Get the sender id of the frame
        :return: sender id
        :rtype: int
        """
        return self.__sender_id

    def __set_receivers_id(self, receivers_id):
        """
        Set the list of receivers id
        :param receivers_id: list of receivers id
        :type receivers_id: list of int
        """
        if not isinstance(receivers_id, list):
            raise TypeError('Receivers id should be a list')
        if not all(isinstance(receiver_id, int) for receiver_id in receivers_id):       # Check if all elements are int
            raise TypeError('All elements in the receivers id list should be integer')
        if any(receiver_id < 0 for receiver_id in receivers_id):                    # If any elements is negative
            raise ValueError('An element is negative, there can not be negative ids')
        self.__receivers_id = receivers_id

    def __get_receivers_id(self):
        """
        Get the list of receivers id
        :return: receivers id
        :rtype: list of int
        """
        return self.__receivers_id

    def __set_size(self, size):
        """
        Set the size of the frame
        :param size: frame size, we follow the Ethernet Standard, so, the payload can be between 64 and 1500 bytes
        :type size: int
        """
        if not isinstance(size, int):
            raise TypeError('The size should be an integer')
        if size < 64 or size > 1500:        # It should be in the Standard Ethernet Payload range
            raise ValueError('The size should be in the range of Standard Ethernet frames [64,1500] bytes')
        self.__size = size

    def __get_size(self):
        """
        Get the size of the frame
        :return: frame size
        :rtype: int
        """
        return self.__size

    def __set_starting_time(self, starting_time):
        """
        Set the starting time of the frame in the schedule in ns
        :param starting_time: starting time in ns
        :type starting_time: int
        """
        if not isinstance(starting_time, int):
            raise TypeError('The starting time should be an integer')
        if starting_time < 0:
            raise ValueError('The starting time should be positive')
        self.__starting_time = starting_time

    def __get_starting_time(self):
        """
        Get the starting time of the frame
        :return: starting time in ns
        :rtype: int
        """
        return self.__starting_time

    def __set_period(self, period):
        """
        Set the period of the frame in ns
        :param period: period in ns
        :type period: int
        """
        if not isinstance(period, int):
            raise TypeError('The period should be an integer')
        if period <= 0:
            raise ValueError('The period should be a natural number')
        self.__period = period

    def __get_period(self):
        """
        Get the period of the frame in ns
        :return: frame period in ns
        :rtype: int
        """
        return self.__period

    def __set_deadline(self, deadline):
        """
        Set the deadline of the frame in ns, it should be smaller than the period, if 0 then same as period
        :param deadline: frame deadline in ns
        :type deadline: int
        """
        if not isinstance(deadline, int):
            raise TypeError('The deadline should be an integer')
        if deadline < 0:
            raise ValueError('The deadline should be positive')
        if deadline > self.__period:
            raise ValueError('The deadline cannot be larger than the period')
        if deadline == 0:
            self.__deadline = self.__period
        else:
            self.__deadline = deadline

    def __get_deadline(self):
        """
        Get the deadline of the frame in ns
        :return: frame deadline in ns
        :rtype: int
        """
        return self.__deadline

    def __set_end_to_end_delay(self, end_to_end):
        """
        Set the end to end delay of the frame in ns
        :param end_to_end: frame end to end delay in ns
        :type end_to_end: int
        """
        if not isinstance(end_to_end, int):
            raise TypeError('The end to end delay should be an integer')
        if end_to_end <= 0:
            raise ValueError('The end to end delay should be positive')
        self.__end_to_end_delay = end_to_end

    def __get_end_to_end_delay(self):
        """
        Get the end to end delays of the frame in ns
        :return: end to end delays of the frame in ns
        :rtype: list of in
        """
        return self.__end_to_end_delay

    sender_id = property(__get_sender_id, __set_sender_id)              # Sender id of the frame
    receivers_id = property(__get_receivers_id, __set_receivers_id)     # List of receivers id of the frame
    size = property(__get_size, __set_size)                             # Ethernet Frame size in bytes [64,1500]
    starting_time = property(__get_starting_time, __set_starting_time)  # Starting time in ns of the frame
    period = property(__get_period, __set_period)                       # Frame period in ns
    deadline = property(__get_deadline, __set_deadline)                 # Frame deadline in ns
    end_to_end_delay = property(__get_end_to_end_delay, __set_end_to_end_delay)     # Frame end to end delay in ns
