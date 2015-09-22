# Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
#
# The information contained herein is property of Nordic Semiconductor ASA.
# Terms and conditions of usage are described in detail in NORDIC
# SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
#
# Licensees are granted free, non-transferable use of the information. NO
# WARRANTY of ANY KIND is provided. This heading must NOT be removed from
# the file.

# Python standard library
import time
from datetime import datetime, timedelta
import abc
import logging

# Nordic libraries
from nordicsemi.exceptions import NordicSemiException, IllegalStateException
from nordicsemi.dfu.util import int16_to_bytes
from nordicsemi.dfu.dfu_transport import DfuTransport, DfuEvent

logging = logging.getLogger(__name__)


# BLE DFU OpCodes :
class DfuOpcodesBle(object):
    """ DFU opcodes used during DFU communication with bootloader

        See http://developer.nordicsemi.com/nRF51_SDK/doc/7.2.0/s110/html/a00949.html#gafa9a52a3e6c43ccf00cf680f944d67a3
        for further information
    """
    START_DFU = 1
    INITIALIZE_DFU = 2
    RECEIVE_FIRMWARE_IMAGE = 3
    VALIDATE_FIRMWARE_IMAGE = 4
    ACTIVATE_FIRMWARE_AND_RESET = 5
    SYSTEM_RESET = 6
    REQ_PKT_RCPT_NOTIFICATION = 8
    RESPONSE = 16
    PKT_RCPT_NOTIF = 17


class DfuErrorCodeBle(object):
    """ DFU error code used during DFU communication with bootloader

        See http://developer.nordicsemi.com/nRF51_SDK/doc/7.2.0/s110/html/a00949.html#gafa9a52a3e6c43ccf00cf680f944d67a3
        for further information
    """
    SUCCESS = 1
    INVALID_STATE = 2
    NOT_SUPPORTED = 3
    DATA_SIZE_EXCEEDS_LIMIT = 4
    CRC_ERROR = 5
    OPERATION_FAILED = 6

    @staticmethod
    def error_code_lookup(error_code):
        """
        Returns a description lookup table for error codes received from peer.

        :param int error_code: Error code to parse
        :return str: Textual description of the error code
        """
        code_lookup = {DfuErrorCodeBle.SUCCESS: "SUCCESS",
                       DfuErrorCodeBle.INVALID_STATE: "Invalid State",
                       DfuErrorCodeBle.NOT_SUPPORTED: "Not Supported",
                       DfuErrorCodeBle.DATA_SIZE_EXCEEDS_LIMIT: "Data Size Exceeds Limit",
                       DfuErrorCodeBle.CRC_ERROR: "CRC Error",
                       DfuErrorCodeBle.OPERATION_FAILED: "Operation Failed"}

        return code_lookup.get(error_code, "UNKOWN ERROR CODE")

# Service UUID. For further information, look at the nRF51 SDK documentation V7.2.0:
# http://developer.nordicsemi.com/nRF51_SDK/doc/7.2.0/s110/html/a00071.html#ota_spec_number
UUID_DFU_SERVICE = '000015301212EFDE1523785FEABCD123'
# Characteristic UUID
UUID_DFU_PACKET_CHARACTERISTIC = '000015321212EFDE1523785FEABCD123'
UUID_DFU_CONTROL_STATE_CHARACTERISTIC = '000015311212EFDE1523785FEABCD123'
# Descriptor UUID
UUID_CLIENT_CHARACTERISTIC_CONFIGURATION_DESCRIPTOR = 0x2902

# NOTE:  If packet receipt notification is enabled, a packet receipt
#        notification will be received for each 'num_of_packets_between_notif'
#        number of packets.
#
# Configuration tip: Increase this to get lesser notifications from the DFU
# Target about packet receipts. Make it 0 to disable the packet receipt
# notification

NUM_OF_PACKETS_BETWEEN_NOTIF = 10
DATA_PACKET_SIZE = 20


class DfuTransportBle(DfuTransport):

    def __init__(self):
        super(DfuTransportBle, self).__init__()
        self.ready_to_send = True
        self.response_opcode_received = None
        self.last_error = DfuErrorCodeBle.SUCCESS
        self.disconnected_event_received = False

    def open(self):
        super(DfuTransportBle, self).open()

    def is_open(self):
        return super(DfuTransportBle, self).is_open()

    def close(self):
        super(DfuTransportBle, self).close()

    def _wait_for_response(self, opcode):
        """
        Waits for self.response_opcode_received to be set to the expected opcode
        Will timeout after 10 seconds

        :param int opcode: The expected opcode
        :return:
        """
        timeout = 40
        start_time = datetime.now()

        while self.response_opcode_received != opcode:
            timed_out = datetime.now() - start_time > timedelta(0, timeout)
            if timed_out:
                log_message = "Timeout while waiting for response from device."
                self._send_event(DfuEvent.TIMEOUT_EVENT, log_message=log_message)
                raise NordicSemiException(log_message)

            if self.disconnected_event_received:
                log_message = "Disconnected from device."
                raise IllegalStateException(log_message)

            time.sleep(0.1)

        if self.last_error != DfuErrorCodeBle.SUCCESS:
            error_message = DfuErrorCodeBle.error_code_lookup(self.last_error)
            self._send_event(DfuEvent.ERROR_EVENT, log_message=error_message)
            raise NordicSemiException(error_message)

        self.response_opcode_received = None

    def _disconnected_event(self, reason):
        self.disconnected_event_received = True

    @abc.abstractmethod
    def send_packet_data(self, data):
        """
        Send data to the packet characteristic

        :param str data: The data to be sent
        :param str log_message: Message describing the data packet
        :return:
        """
        pass

    @abc.abstractmethod
    def send_control_data(self, opcode, data=""):
        """
        Send data to the control characteristic

        :param int opcode: The opcode for the operation command sent to the control characteristic
        :param str data: The data to be sent
        :param str log_message: Message describing the control packet
        :return:
        """
        pass

    def send_start_dfu(self, program_mode, softdevice_size=0, bootloader_size=0, app_size=0):
        super(DfuTransportBle, self).send_start_dfu(program_mode, softdevice_size, bootloader_size, app_size)
        image_size_packet = DfuTransport.create_image_size_packet(softdevice_size, bootloader_size, app_size)

        self._send_event(DfuEvent.PROGRESS_EVENT, progress=0, log_message="Setting up transfer...")

        try:
            logging.debug("Sending 'START DFU' command")
            self.send_control_data(DfuOpcodesBle.START_DFU, chr(program_mode))
            logging.debug("Sending image size")
            self.send_packet_data(image_size_packet)
            self._wait_for_response(DfuOpcodesBle.START_DFU)
        except IllegalStateException:
            #If at first you don't succeed, try, try again.
            self.close()
            self.disconnected_event_received = False
            self.open()

            if not self.is_open():
                raise IllegalStateException("Failed to open transport backend.")

            logging.debug("Sending 'START DFU' command")
            self.send_control_data(DfuOpcodesBle.START_DFU, chr(program_mode))
            logging.debug("Sending image size")
            self.send_packet_data(image_size_packet)
            self._wait_for_response(DfuOpcodesBle.START_DFU)

    def send_init_packet(self, init_packet):
        super(DfuTransportBle, self).send_init_packet(init_packet)
        init_packet_start = chr(0x00)
        init_packet_end = chr(0x01)

        logging.debug("Sending 'INIT DFU' command")
        self.send_control_data(DfuOpcodesBle.INITIALIZE_DFU, init_packet_start)

        logging.debug("Sending init data")
        for i in range(0, len(init_packet), DATA_PACKET_SIZE):
            data_to_send = init_packet[i:i + DATA_PACKET_SIZE]
            self.send_packet_data(data_to_send)

        logging.debug("Sending 'Init Packet Complete' command")
        self.send_control_data(DfuOpcodesBle.INITIALIZE_DFU, init_packet_end)
        self._wait_for_response(DfuOpcodesBle.INITIALIZE_DFU)

        if NUM_OF_PACKETS_BETWEEN_NOTIF:
            packet = int16_to_bytes(NUM_OF_PACKETS_BETWEEN_NOTIF)
            logging.debug("Send number of packets before device sends notification")
            self.send_control_data(DfuOpcodesBle.REQ_PKT_RCPT_NOTIFICATION, packet)

    def send_firmware(self, firmware):
        def progress_percentage(part, complete):
            """
                Calculate progress percentage
                :param part: Part value
                :type part: int
                :param complete: Completed value
                :type complete: int
                :return: Percentage complete
                :rtype: int
                """
            return min(100, (part + DATA_PACKET_SIZE) * 100 / complete)

        def sleep_until_ready_to_send():
            """
            Waits until a notification is received from peer device
            Will timeout after 10 seconds

            :return:
            """
            if NUM_OF_PACKETS_BETWEEN_NOTIF:
                timeout = 10
                start_time = datetime.now()

                while not self.ready_to_send:
                    timed_out = datetime.now() - start_time > timedelta(0, timeout)
                    if timed_out:
                        log_message = "Timeout while waiting for notification from device."
                        self._send_event(DfuEvent.TIMEOUT_EVENT, log_message=log_message)
                        raise NordicSemiException(log_message)

                    time.sleep(0.1)

        super(DfuTransportBle, self).send_firmware(firmware)
        packets_sent = 0
        last_progress_update = -1  # Last packet sequence number when an update was fired to the event system
        bin_size = len(firmware)
        logging.debug("Send 'RECEIVE FIRMWARE IMAGE' command")
        self.send_control_data(DfuOpcodesBle.RECEIVE_FIRMWARE_IMAGE)

        for i in range(0, bin_size, DATA_PACKET_SIZE):
            progress = progress_percentage(i, bin_size)

            if progress != last_progress_update:
                self._send_event(DfuEvent.PROGRESS_EVENT, progress=progress, log_message="Uploading firmware")
                last_progress_update = progress

            sleep_until_ready_to_send()

            data_to_send = firmware[i:i + DATA_PACKET_SIZE]

            log_message = "Sending Firmware bytes [{0}, {1}]".format(i, i + len(data_to_send))
            logging.debug(log_message)
            self.send_packet_data(data_to_send)

            packets_sent += 1

            if NUM_OF_PACKETS_BETWEEN_NOTIF != 0:
                if (packets_sent % NUM_OF_PACKETS_BETWEEN_NOTIF) == 0:
                    self.ready_to_send = False

        self._wait_for_response(DfuOpcodesBle.RECEIVE_FIRMWARE_IMAGE)

    def send_validate_firmware(self):
        super(DfuTransportBle, self).send_validate_firmware()
        logging.debug("Sending 'VALIDATE FIRMWARE IMAGE' command")
        self.send_control_data(DfuOpcodesBle.VALIDATE_FIRMWARE_IMAGE)
        self._wait_for_response(DfuOpcodesBle.VALIDATE_FIRMWARE_IMAGE)
        logging.info("Firmware validated OK.")

    def send_activate_firmware(self):
        super(DfuTransportBle, self).send_activate_firmware()
        logging.debug("Sending 'ACTIVATE FIRMWARE AND RESET' command")
        self.send_control_data(DfuOpcodesBle.ACTIVATE_FIRMWARE_AND_RESET)
